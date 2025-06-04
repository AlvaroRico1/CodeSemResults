bool JOIN::optimize() {
  DBUG_TRACE;

  uint no_jbuf_after = UINT_MAX;

  assert(query_block->leaf_table_count == 0 ||
         thd->lex->is_query_tables_locked() ||
         query_block == query_expression()->fake_query_block);
  assert(tables == 0 && primary_tables == 0 && tables_list == (TABLE_LIST *)1);

  // to prevent double initialization on EXPLAIN
  if (optimized) return false;

  DEBUG_SYNC(thd, "before_join_optimize");

  THD_STAGE_INFO(thd, stage_optimizing);

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_optimize(trace, "join_optimization");
  trace_optimize.add_select_number(query_block->select_number);
  Opt_trace_array trace_steps(trace, "steps");

  count_field_types(query_block, &tmp_table_param, *fields, false, false);

  assert(tmp_table_param.sum_func_count == 0 || !group_list.empty() ||
         implicit_grouping);

  const bool has_windows = m_windows.elements != 0;

  if (has_windows && Window::setup_windows2(thd, m_windows))
    return true; /* purecov: inspected */

  if (query_block->olap == ROLLUP_TYPE && optimize_rollup())
    return true; /* purecov: inspected */

  if (alloc_func_list()) return true; /* purecov: inspected */

  if (query_block->get_optimizable_conditions(thd, &where_cond, &having_cond))
    return true;

  for (Item_rollup_group_item *item : query_block->rollup_group_items) {
    rollup_group_items.push_back(item);
  }
  for (Item_rollup_sum_switcher *item : query_block->rollup_sums) {
    rollup_sums.push_back(item);
  }

  set_optimized();

  tables_list = query_block->leaf_tables;

  if (alloc_indirection_slices()) return true;

  // The base ref items from query block are assigned as JOIN's ref items
  ref_items[REF_SLICE_ACTIVE] = query_block->base_ref_items;

  /* dump_TABLE_LIST_graph(query_block, query_block->leaf_tables); */
  /*
    Run optimize phase for all derived tables/views used in this SELECT,
    including those in semi-joins.
  */
  // if (query_block->materialized_derived_table_count) {
  {  // WL#6570
    for (TABLE_LIST *tl = query_block->leaf_tables; tl; tl = tl->next_leaf) {
      if (tl->is_view_or_derived()) {
        if (tl->optimize_derived(thd)) return true;
      } else if (tl->is_table_function()) {
        TABLE *const table = tl->table;
        if (!table->has_storage_handler()) {
          if (setup_tmp_table_handler(
                  thd, table,
                  query_block->active_options() | TMP_TABLE_ALL_COLUMNS))
            return true; /* purecov: inspected */
        }

        table->file->stats.records = 2;
      }
    }
  }

  has_lateral = false;

  /* dump_TABLE_LIST_graph(query_block, query_block->leaf_tables); */

  row_limit = ((select_distinct || !order.empty() || !group_list.empty())
                   ? HA_POS_ERROR
                   : query_expression()->select_limit_cnt);
  // m_select_limit is used to decide if we are likely to scan the whole table.
  m_select_limit = query_expression()->select_limit_cnt;

  if (query_expression()->first_query_block()->active_options() &
      OPTION_FOUND_ROWS) {
    /*
      Calculate found rows (ie., keep counting rows even after we hit LIMIT) if
      - LIMIT is set, and
      - This is the outermost query block (for a UNION query, this is the
        fake query block that contains the limit applied on the final UNION
        evaluation).
     */
    calc_found_rows = m_select_limit != HA_POS_ERROR &&
                      (!query_expression()->is_union() ||
                       query_block == query_expression()->fake_query_block);
  }
  if (having_cond || calc_found_rows) m_select_limit = HA_POS_ERROR;

  if (query_expression()->select_limit_cnt == 0 && !calc_found_rows) {
    zero_result_cause = "Zero limit";
    best_rowcount = 0;
    create_access_paths_for_zero_rows();
    goto setup_subq_exit;
  }

  if (where_cond || query_block->outer_join) {
    if (optimize_cond(thd, &where_cond, &cond_equal,
                      &query_block->top_join_list, &query_block->cond_value)) {
      error = 1;
      DBUG_PRINT("error", ("Error from optimize_cond"));
      return true;
    }
    if (query_block->cond_value == Item::COND_FALSE) {
      zero_result_cause = "Impossible WHERE";
      best_rowcount = 0;
      create_access_paths_for_zero_rows();
      goto setup_subq_exit;
    }
  }
  if (having_cond) {
    if (optimize_cond(thd, &having_cond, &cond_equal, nullptr,
                      &query_block->having_value)) {
      error = 1;
      DBUG_PRINT("error", ("Error from optimize_cond"));
      return true;
    }
    if (query_block->having_value == Item::COND_FALSE) {
      zero_result_cause = "Impossible HAVING";
      best_rowcount = 0;
      create_access_paths_for_zero_rows();
      goto setup_subq_exit;
    }
  }

  if (query_block->partitioned_table_count && prune_table_partitions()) {
    error = 1;
    DBUG_PRINT("error", ("Error from prune_partitions"));
    return true;
  }

  /*
     Try to optimize count(*), min() and max() to const fields if
     there is implicit grouping (aggregate functions but no
     group_list). In this case, the result set shall only contain one
     row.
  */
  if (tables_list && implicit_grouping &&
      !(query_block->active_options() & OPTION_NO_CONST_TABLES)) {
    aggregate_evaluated outcome;
    if (optimize_aggregated_query(thd, query_block, *fields, where_cond,
                                  &outcome)) {
      error = 1;
      DBUG_PRINT("error", ("Error from optimize_aggregated_query"));
      return true;
    }
    switch (outcome) {
      case AGGR_REGULAR:
        // Query was not (fully) evaluated. Revert to regular optimization.
        break;
      case AGGR_DELAYED:
        // Query was not (fully) evaluated. Revert to regular optimization,
        // but indicate that storage engine supports HA_COUNT_ROWS_INSTANT.
        select_count = true;
        break;
      case AGGR_COMPLETE: {
        // All SELECT expressions are fully evaluated
        DBUG_PRINT("info", ("Select tables optimized away"));
        zero_result_cause = "Select tables optimized away";
        tables_list = nullptr;  // All tables resolved
        best_rowcount = 1;
        const_tables = tables = primary_tables = query_block->leaf_table_count;
        AccessPath *path =
            NewFakeSingleRowAccessPath(thd, /*count_examined_rows=*/true);
        path = attach_access_paths_for_having_and_limit(path);
        m_root_access_path = path;
        /*
          There are no relevant conditions left from the WHERE;
          optimize_aggregated_query() will not return AGGR_COMPLETE if there are
          any table-independent conditions, and all other conditions have been
          optimized away by it. Thus, remove the condition, unless we have
          EXPLAIN (in which case we will keep it for printing).
        */
        if (!thd->lex->is_explain()) {
#ifndef NDEBUG
          // Verify, to be sure.
          if (where_cond != nullptr) {
            Item *table_independent_conds = make_cond_for_table(
                thd, where_cond, PSEUDO_TABLE_BITS, table_map(0), false);
            assert(table_independent_conds == nullptr);
          }
#endif
          where_cond = nullptr;
        }
        goto setup_subq_exit;
      }
      case AGGR_EMPTY:
        // It was detected that the result tables are empty
        DBUG_PRINT("info", ("No matching min/max row"));
        zero_result_cause = "No matching min/max row";
        create_access_paths_for_zero_rows();
        goto setup_subq_exit;
    }
  }
  if (tables_list == nullptr) {
    DBUG_PRINT("info", ("No tables"));
    best_rowcount = 1;
    error = 0;
    if (make_tmp_tables_info()) return true;
    count_field_types(query_block, &tmp_table_param, *fields, false, false);
    // Make plan visible for EXPLAIN
    set_plan_state(NO_TABLES);
    create_access_paths();
    return false;
  }
  error = -1;  // Error is sent to client

  {
    m_windowing_steps = false;  // initialization
    m_windows_sort = false;
    List_iterator<Window> li(m_windows);
    Window *w;
    while ((w = li++))
      if (w->needs_sorting()) {
        m_windows_sort = true;
        break;
      }
  }

  sort_by_table = get_sort_by_table(order.order, group_list.order,
                                    query_block->leaf_tables);

  if ((where_cond || !group_list.empty() || !order.empty()) &&
      substitute_gc(thd, query_block, where_cond, group_list.order,
                    order.order)) {
    // We added hidden fields to the all_fields list, count them.
    count_field_types(query_block, &tmp_table_param, query_block->fields, false,
                      false);
  }
  // Ensure there are no errors prior making query plan
  if (thd->is_error()) return true;

  if (thd->lex->using_hypergraph_optimizer) {
    if (thd->opt_trace.is_started()) {
      std::string trace_str;
      m_root_access_path = FindBestQueryPlan(thd, query_block, &trace_str);
      Opt_trace_object trace_wrapper2(&thd->opt_trace);
      Opt_trace_array join_optimizer(&thd->opt_trace, "join_optimizer");

      // Split by newlines.
      for (size_t pos = 0; pos < trace_str.size();) {
        size_t len = strcspn(trace_str.data() + pos, "\n");
        join_optimizer.add_utf8(trace_str.data() + pos, len);
        pos += len + 1;
      }
    } else {
      m_root_access_path =
          FindBestQueryPlan(thd, query_block, /*trace=*/nullptr);
    }
    if (m_root_access_path == nullptr) {
      return true;
    }
    set_plan_state(PLAN_READY);
    DEBUG_SYNC(thd, "after_join_optimize");
    return false;
  }

  // ----------------------------------------------------------------------------
  //       All of this is never called for the hypergraph join optimizer!
  // ----------------------------------------------------------------------------

  // Set up join order and initial access paths
  THD_STAGE_INFO(thd, stage_statistics);
  if (make_join_plan()) {
    if (thd->killed) thd->send_kill_message();
    DBUG_PRINT("error", ("Error: JOIN::make_join_plan() failed"));
    return true;
  }

  // At this stage, join_tab==NULL, JOIN_TABs are listed in order by best_ref.
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  if (zero_result_cause != nullptr) {  // Can be set by make_join_plan().
    create_access_paths_for_zero_rows();
    goto setup_subq_exit;
  }

  if (rollup_state == RollupState::NONE) {
    /* Remove distinct if only const tables */
    select_distinct &= !plan_is_const();
  }

  if (const_tables && !thd->locked_tables_mode &&
      !(query_block->active_options() & SELECT_NO_UNLOCK)) {
    TABLE *ct[MAX_TABLES];
    for (uint i = 0; i < const_tables; i++) {
      ct[i] = best_ref[i]->table();
      ct[i]->file->ha_index_or_rnd_end();
    }
    mysql_unlock_some_tables(thd, ct, const_tables);
  }
  if (!where_cond && query_block->outer_join) {
    /* Handle the case where we have an OUTER JOIN without a WHERE */
    where_cond = new Item_func_true();  // Always true
  }

  error = 0;
  /*
    Among the equal fields belonging to the same multiple equality
    choose the one that is to be retrieved first and substitute
    all references to these in where condition for a reference for
    the selected field.
  */
  if (where_cond) {
    where_cond =
        substitute_for_best_equal_field(thd, where_cond, cond_equal, map2table);
    if (thd->is_error()) {
      error = 1;
      DBUG_PRINT("error", ("Error from substitute_for_best_equal"));
      return true;
    }
    where_cond->update_used_tables();
    DBUG_EXECUTE("where",
                 print_where(thd, where_cond, "after substitute_best_equal",
                             QT_ORDINARY););
  }

  /*
    Perform the same optimization on field evaluation for all join conditions.
  */
  for (uint i = const_tables; i < tables; ++i) {
    JOIN_TAB *const tab = best_ref[i];
    if (tab->position() && tab->join_cond()) {
      tab->set_join_cond(substitute_for_best_equal_field(
          thd, tab->join_cond(), tab->cond_equal, map2table));
      if (thd->is_error()) {
        error = 1;
        DBUG_PRINT("error", ("Error from substitute_for_best_equal"));
        return true;
      }
      tab->join_cond()->update_used_tables();
      if (tab->join_cond())
        tab->join_cond()->walk(&Item::cast_incompatible_args,
                               enum_walk::POSTFIX, nullptr);
    }
  }

  if (init_ref_access()) {
    error = 1;
    DBUG_PRINT("error", ("Error from init_ref_access"));
    return true;
  }

  // Update table dependencies after assigning ref access fields
  update_depend_map();

  THD_STAGE_INFO(thd, stage_preparing);

  if (make_join_query_block(this, where_cond)) {
    if (thd->is_error()) return true;

    zero_result_cause = "Impossible WHERE noticed after reading const tables";
    create_access_paths_for_zero_rows();
    goto setup_subq_exit;
  }

  // Inject cast nodes into the WHERE conditions
  if (where_cond)
    where_cond->walk(&Item::cast_incompatible_args, enum_walk::POSTFIX,
                     nullptr);

  error = -1; /* if goto err */

  if (optimize_distinct_group_order()) return true;

  if ((query_block->active_options() & SELECT_NO_JOIN_CACHE) ||
      query_block->ftfunc_list->elements)
    no_jbuf_after = 0;

  /* Perform FULLTEXT search before all regular searches */
  if (query_block->has_ft_funcs() && optimize_fts_query()) return true;

  /*
    By setting child_subquery_can_materialize so late we gain the following:
    JOIN::compare_costs_of_subquery_strategies() can test this variable to
    know if we are have finished evaluating constant conditions, which itself
    helps determining fanouts.
  */
  child_subquery_can_materialize = true;

  /*
    It's necessary to check const part of HAVING cond as
    there is a chance that some cond parts may become
    const items after make_join_plan() (for example
    when Item is a reference to const table field from
    outer join).
    This check is performed only for those conditions
    which do not use aggregate functions. In such case
    temporary table may not be used and const condition
    elements may be lost during further having
    condition transformation in JOIN::exec.
  */
  if (having_cond && !having_cond->has_aggregation() && (const_tables > 0)) {
    having_cond->update_used_tables();
    if (remove_eq_conds(thd, having_cond, &having_cond,
                        &query_block->having_value)) {
      error = 1;
      DBUG_PRINT("error", ("Error from remove_eq_conds"));
      return true;
    }
    if (query_block->having_value == Item::COND_FALSE) {
      having_cond = new Item_func_false();
      zero_result_cause =
          "Impossible HAVING noticed after reading const tables";
      create_access_paths_for_zero_rows();
      goto setup_subq_exit;
    }
  }

  // Inject cast nodes into the HAVING conditions
  if (having_cond)
    having_cond->walk(&Item::cast_incompatible_args, enum_walk::POSTFIX,
                      nullptr);

  // Traverse the expressions and inject cast nodes to compatible data types,
  // if needed.
  for (Item *item : *fields) {
    item->walk(&Item::cast_incompatible_args, enum_walk::POSTFIX, nullptr);
  }

  if (rollup_state != RollupState::NONE) {
    /*
      Fields may have been replaced by Item_rollup_group_item, so
      recalculate the number of fields and functions for this query block.
    */

    // JOIN::optimize_rollup() may set allow_group_via_temp_table = false,
    // and we must not undo that.
    const bool save_allow_group_via_temp_table =
        tmp_table_param.allow_group_via_temp_table;

    count_field_types(query_block, &tmp_table_param, *fields, false, false);
    tmp_table_param.allow_group_via_temp_table =
        save_allow_group_via_temp_table;
  }

  // See if this subquery can be evaluated with subselect_indexsubquery_engine
  if (const int ret = replace_index_subquery()) {
    if (ret == -1) {
      // Error (e.g. allocation failed, or some condition was attempted
      // evaluated statically and failed).
      return true;
    }

    create_access_paths_for_index_subquery();
    set_plan_state(PLAN_READY);
    /*
      We leave optimize() because the rest of it is only about order/group
      which those subqueries don't have and about setting up plan which
      we're not going to use due to different execution method.
    */
    return false;
  }

  {
    /*
      If the hint FORCE INDEX FOR ORDER BY/GROUP BY is used for the first
      table (it does not make sense for other tables) then we cannot do join
      buffering.
    */
    if (!plan_is_const()) {
      const TABLE *const first = best_ref[const_tables]->table();
      if ((first->force_index_order && !order.empty()) ||
          (first->force_index_group && !group_list.empty()))
        no_jbuf_after = 0;
    }

    bool simple_sort = true;
    Deps_of_remaining_lateral_derived_tables deps_lateral(this, all_table_map);
    // Check whether join cache could be used
    for (uint i = const_tables; i < tables; i++) {
      JOIN_TAB *const tab = best_ref[i];
      if (!tab->position()) continue;
      if (setup_join_buffering(tab, this, no_jbuf_after)) return true;
      if (tab->use_join_cache() != JOIN_CACHE::ALG_NONE) simple_sort = false;
      assert(tab->type() != JT_FT ||
             tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
      deps_lateral.recalculate(tab, i + 1);
    }
    if (!simple_sort) {
      /*
        A join buffer is used for this table. We here inform the optimizer
        that it should not rely on rows of the first non-const table being in
        order thanks to an index scan; indeed join buffering of the present
        table subsequently changes the order of rows.
      */
      simple_order = simple_group = false;
    }
  }

  if (!plan_is_const() && !order.empty()) {
    /*
      Force using of tmp table if sorting by a SP or UDF function due to
      their expensive and probably non-deterministic nature.
    */
    for (ORDER *tmp_order = order.order; tmp_order;
         tmp_order = tmp_order->next) {
      Item *item = *tmp_order->item;
      if (item->is_expensive()) {
        /* Force tmp table without sort */
        simple_order = simple_group = false;
        break;
      }
    }
  }


// Source: sql_optimizer.cc
// Lines 255-776
