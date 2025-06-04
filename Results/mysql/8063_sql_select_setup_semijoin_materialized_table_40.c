bool JOIN::setup_semijoin_materialized_table(JOIN_TAB *tab, uint tableno,
                                             POSITION *inner_pos,
                                             POSITION *sjm_pos) {
  DBUG_TRACE;
  TABLE_LIST *const emb_sj_nest = inner_pos->table->emb_sj_nest;
  Semijoin_mat_optimize *const sjm_opt = &emb_sj_nest->nested_join->sjm;
  Semijoin_mat_exec *const sjm_exec = tab->sj_mat_exec();
  const uint field_count = emb_sj_nest->nested_join->sj_inner_exprs.size();

  assert(field_count > 0);
  assert(inner_pos->sj_strategy == SJ_OPT_MATERIALIZE_LOOKUP ||
         inner_pos->sj_strategy == SJ_OPT_MATERIALIZE_SCAN);

  /*
    Set up the table to write to, do as
    Query_result_union::create_result_table does
  */
  sjm_exec->table_param = Temp_table_param();
  count_field_types(query_block, &sjm_exec->table_param,
                    emb_sj_nest->nested_join->sj_inner_exprs, false, true);
  sjm_exec->table_param.bit_fields_as_long = true;

  char buffer[NAME_LEN];
  const size_t len = snprintf(buffer, sizeof(buffer) - 1, "<subquery%u>",
                              emb_sj_nest->nested_join->query_block_id);
  char *name = (char *)thd->mem_root->Alloc(len + 1);
  if (name == nullptr) return true; /* purecov: inspected */

  memcpy(name, buffer, len);
  name[len] = '\0';
  TABLE *table;
  if (!(table =
            create_tmp_table(thd, &sjm_exec->table_param,
                             emb_sj_nest->nested_join->sj_inner_exprs, nullptr,
                             true /* distinct */, true /* save_sum_fields */,
                             thd->variables.option_bits | TMP_TABLE_ALL_COLUMNS,
                             HA_POS_ERROR /* rows_limit */, name)))
    return true; /* purecov: inspected */
  sjm_exec->table = table;
  map2table[tableno] = tab;
  table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
  sj_tmp_tables.push_back(table);
  sjm_exec_list.push_back(sjm_exec);

  /*
    Hash_field is not applicable for MATERIALIZE_LOOKUP. If hash_field is
    created for temporary table, semijoin_types_allow_materialization must
    assure that MATERIALIZE_LOOKUP can't be chosen.
  */
  assert((inner_pos->sj_strategy == SJ_OPT_MATERIALIZE_LOOKUP &&
          !table->hash_field) ||
         inner_pos->sj_strategy == SJ_OPT_MATERIALIZE_SCAN);

  auto tl = new (thd->mem_root) TABLE_LIST("", name, TL_IGNORE);
  if (tl == nullptr) return true; /* purecov: inspected */
  tl->table = table;

  /*
    If the SJ nest is inside an outer join nest, this tmp table belongs to
    it. It's important for attachment of the semi-join ON condition with the
    proper guards, to this table. If it's an AJ nest it's an outer join
    nest too.
  */
  if (emb_sj_nest->is_aj_nest())
    tl->embedding = emb_sj_nest;
  else
    tl->embedding = emb_sj_nest->outer_join_nest();
  /*
    Above, we do not set tl->emb_sj_nest, neither first_sj_inner nor
    last_sj_inner; it's because there's no use to say that this table is part
    of the SJ nest; but it's necessary to say that it's part of any outer join
    nest. The antijoin nest is an outer join nest, but from the POV of the
    sj-tmp table it's only an outer join nest, so there is no need to set
    emb_sj_nest even in this case.
  */

  // Table is "nullable" if inner table of an outer_join
  if (tl->is_inner_table_of_outer_join()) table->set_nullable();

  tl->set_tableno(tableno);

  table->pos_in_table_list = tl;
  table->pos_in_table_list->query_block = query_block;

  if (!(sjm_opt->mat_fields = (Item_field **)thd->mem_root->Alloc(
            field_count * sizeof(Item_field **))))
    return true;

  for (uint fieldno = 0; fieldno < field_count; fieldno++) {
    if (!(sjm_opt->mat_fields[fieldno] =
              new Item_field(table->visible_field_ptr()[fieldno])))
      return true;
  }

  tab->table_ref = tl;
  tab->set_table(table);
  tab->set_position(sjm_pos);

  tab->worst_seeks = 1.0;
  tab->set_records((ha_rows)emb_sj_nest->nested_join->sjm.expected_rowcount);

  tab->found_records = tab->records();
  tab->read_time = emb_sj_nest->nested_join->sjm.scan_cost.total_cost();

  tab->init_join_cond_ref(tl);

  table->keys_in_use_for_query.set_all();
  sjm_pos->table = tab;
  sjm_pos->sj_strategy = SJ_OPT_NONE;

  sjm_pos->use_join_buffer = false;
  /*
    No need to recalculate filter_effect since there are no post-read
    conditions for materialized tables.
  */
  sjm_pos->filter_effect = 1.0;

  /*
    Key_use objects are required so that create_ref_for_key() can set up
    a proper ref access for this table.
  */
  Key_use_array *keyuse =
      create_keyuse_for_table(thd, field_count, sjm_opt->mat_fields,
                              emb_sj_nest->nested_join->sj_outer_exprs);
  if (!keyuse) return true;

  double fanout = ((uint)tab->idx() == const_tables)
                      ? 1.0
                      : best_ref[tab->idx() - 1]->position()->prefix_rowcount;
  if (!sjm_exec->is_scan) {
    sjm_pos->key = keyuse->begin();  // MaterializeLookup will use the index
    sjm_pos->read_cost =
        emb_sj_nest->nested_join->sjm.lookup_cost.total_cost() * fanout;
    tab->set_keyuse(keyuse->begin());
    tab->keys().set_bit(0);  // There is one index - use it always
    tab->set_index(0);
    sjm_pos->rows_fetched = 1.0;
    tab->set_type(JT_REF);
  } else {
    sjm_pos->key = nullptr;  // No index use for MaterializeScan
    sjm_pos->read_cost = tab->read_time * fanout;
    sjm_pos->rows_fetched = static_cast<double>(tab->records());
    tab->set_type(JT_ALL);
  }
  sjm_pos->set_prefix_join_cost((tab - join_tab), cost_model());

  return false;
}

/**
  A helper function that sets the right op type for join cache (BNL/BKA).
*/

void QEP_TAB::init_join_cache(JOIN_TAB *join_tab) {
  assert(idx() > 0);
  ASSERT_BEST_REF_IN_JOIN_ORDER(join());
  assert(join_tab == join()->best_ref[idx()]);

  switch (join_tab->use_join_cache()) {
    case JOIN_CACHE::ALG_BNL:
      op_type = QEP_TAB::OT_BNL;
      break;
    case JOIN_CACHE::ALG_BKA:
      op_type = QEP_TAB::OT_BKA;
      break;
    default:
      assert(0);
  }
}

/**
  Plan refinement stage: do various setup things for the executor

  @param join          Join being processed
  @param no_jbuf_after Don't use join buffering after table with this number.

  @return false if successful, true if error (Out of memory)

  @details
    Plan refinement stage: do various set ups for the executioner
      - setup join buffering use
      - push index conditions
      - increment relevant counters
      - etc
*/

bool make_join_readinfo(JOIN *join, uint no_jbuf_after) {
  const bool statistics = !join->thd->lex->is_explain();
  const bool prep_for_pos = join->need_tmp_before_win ||
                            join->select_distinct ||
                            !join->group_list.empty() || !join->order.empty() ||
                            join->m_windows.elements > 0;

  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);

  Opt_trace_context *const trace = &join->thd->opt_trace;
  Opt_trace_object wrapper(trace);
  Opt_trace_array trace_refine_plan(trace, "refine_plan");

  if (setup_semijoin_dups_elimination(join, no_jbuf_after))
    return true; /* purecov: inspected */

  for (uint i = join->const_tables; i < join->tables; i++) {
    QEP_TAB *const qep_tab = &join->qep_tab[i];
    if (!qep_tab->position()) continue;

    JOIN_TAB *const tab = join->best_ref[i];
    TABLE *const table = qep_tab->table();
    TABLE_LIST *const table_ref = qep_tab->table_ref;
    /*
     Need to tell handlers that to play it safe, it should fetch all
     columns of the primary key of the tables: this is because MySQL may
     build row pointers for the rows, and for all columns of the primary key
     the read set has not necessarily been set by the server code.
    */
    if (prep_for_pos) table->prepare_for_position();

    qep_tab->cache_idx_cond = nullptr;

    Opt_trace_object trace_refine_table(trace);
    trace_refine_table.add_utf8_table(table_ref);

    if (tab->use_join_cache() != JOIN_CACHE::ALG_NONE)
      qep_tab->init_join_cache(tab);

    switch (qep_tab->type()) {
      case JT_EQ_REF:
      case JT_REF_OR_NULL:
      case JT_REF:
      case JT_SYSTEM:
      case JT_CONST:
        if (table->covering_keys.is_set(qep_tab->ref().key) &&
            !table->no_keyread)
          table->set_keyread(true);
        else
          qep_tab->push_index_cond(tab, qep_tab->ref().key,
                                   &trace_refine_table);
        break;
      case JT_ALL:
        join->thd->set_status_no_index_used();
        qep_tab->using_dynamic_range = (tab->use_quick == QS_DYNAMIC_RANGE);
      /* Fall through */
      case JT_INDEX_SCAN:
        if (tab->position()->filter_effect != COND_FILTER_STALE_NO_CONST &&
            !tab->sj_mat_exec()) {
          /*
            rows_w_const_cond is # of rows which will be read by the access
            method, minus those which will not pass the constant condition;
            that's how calculate_scan_cost() works. Such number is useful inside
            the planner, but obscure to the reader of EXPLAIN; so we put the
            real count of read rows into rows_fetched, and move the constant
            condition's filter to filter_effect.
          */
          double rows_w_const_cond = qep_tab->position()->rows_fetched;
          table_ref->fetch_number_of_rows();
          tab->position()->rows_fetched =
              static_cast<double>(table->file->stats.records);
          if (tab->position()->filter_effect != COND_FILTER_STALE) {
            // Constant condition moves to filter_effect:
            if (tab->position()->rows_fetched == 0)  // avoid division by zero
              tab->position()->filter_effect = 0.0f;
            else
              tab->position()->filter_effect *= static_cast<float>(
                  rows_w_const_cond / tab->position()->rows_fetched);
          }
        }
        if (qep_tab->using_dynamic_range) {
          join->thd->set_status_no_good_index_used();
          if (statistics) join->thd->inc_status_select_range_check();
        } else {
          if (statistics) {
            if (i == join->const_tables)
              join->thd->inc_status_select_scan();
            else
              join->thd->inc_status_select_full_join();
          }
        }
        break;
      case JT_RANGE:
      case JT_INDEX_MERGE:
        qep_tab->using_dynamic_range = (tab->use_quick == QS_DYNAMIC_RANGE);
        if (statistics) {
          if (i == join->const_tables)
            join->thd->inc_status_select_range();
          else
            join->thd->inc_status_select_full_range_join();
        }
        if (!table->no_keyread && qep_tab->type() == JT_RANGE) {
          if (table->covering_keys.is_set(qep_tab->quick()->index)) {
            assert(qep_tab->quick()->index != MAX_KEY);
            table->set_keyread(true);
          }
          if (!table->key_read)
            qep_tab->push_index_cond(tab, qep_tab->quick()->index,
                                     &trace_refine_table);
        }
        if (tab->position()->filter_effect != COND_FILTER_STALE_NO_CONST) {
          double rows_w_const_cond = qep_tab->position()->rows_fetched;
          qep_tab->position()->rows_fetched =
              rows2double(tab->quick()->records);
          if (tab->position()->filter_effect != COND_FILTER_STALE) {
            // Constant condition moves to filter_effect:
            if (tab->position()->rows_fetched == 0)  // avoid division by zero
              tab->position()->filter_effect = 0.0f;
            else
              tab->position()->filter_effect *= static_cast<float>(
                  rows_w_const_cond / tab->position()->rows_fetched);
          }
        }
        break;
      case JT_FT:
        if (tab->join()->fts_index_access(tab)) {
          table->set_keyread(true);
          table->covering_keys.set_bit(tab->ft_func()->key);
        }
        break;
      default:
        DBUG_PRINT("error", ("Table type %d found",
                             qep_tab->type())); /* purecov: deadcode */
        assert(0);
        break; /* purecov: deadcode */
    }

    // Now that we have decided which index to use and whether to use "dynamic
    // range scan", it is time to filter away base columns for virtual generated
    // columns from the read_set. This is so that if we are scanning on a
    // covering index, code that uses the table's read set (join buffering, hash
    // join, filesort; they all use it to figure out which records to pack into
    // their buffers) do not try to pack the non-existent base columns. See
    // filter_virtual_gcol_base_cols().
    filter_virtual_gcol_base_cols(qep_tab);

    if (tab->position()->filter_effect <= COND_FILTER_STALE) {
      /*
        Give a proper value for EXPLAIN.
        For performance reasons, we do not recalculate the filter for
        non-EXPLAIN queries; thus, EXPLAIN CONNECTION may show 100%
        for a query.
      */
      tab->position()->filter_effect =
          join->thd->lex->is_explain()
              ? calculate_condition_filter(
                    tab,
                    (tab->ref().key != -1) ? tab->position()->key : nullptr,
                    tab->prefix_tables() & ~table_ref->map(),
                    tab->position()->rows_fetched, false, false,
                    trace_refine_table)
              : COND_FILTER_ALLPASS;
    }

    assert(!table_ref->is_recursive_reference() || qep_tab->type() == JT_ALL);

    qep_tab->set_reversed_access(tab->reversed_access);

    // Materialize derived tables prior to accessing them.
    if (table_ref->is_table_function()) {
      qep_tab->materialize_table = QEP_TAB::MATERIALIZE_TABLE_FUNCTION;
      if (tab->dependent) qep_tab->rematerialize = true;
    } else if (table_ref->uses_materialization()) {
      qep_tab->materialize_table = QEP_TAB::MATERIALIZE_DERIVED;
    }

    if (qep_tab->sj_mat_exec())
      qep_tab->materialize_table = QEP_TAB::MATERIALIZE_SEMIJOIN;

    if (table_ref->is_derived() &&
        table_ref->derived_query_expression()->m_lateral_deps) {
      auto deps = table_ref->derived_query_expression()->m_lateral_deps;
      plan_idx last = NO_PLAN_IDX;
      for (JOIN_TAB **tab2 = join->map2table; deps; tab2++, deps >>= 1) {
        if (deps & 1) last = std::max(last, (*tab2)->idx());
      }
      /*
        We identified the last dependency of table_ref in the plan, and it's
        the table whose reading must trigger rematerialization of table_ref.
      */
      if (last != NO_PLAN_IDX) {
        QEP_TAB &t = join->qep_tab[last];
        t.lateral_derived_tables_depend_on_me |= table_ref->map();
        trace_refine_table.add_utf8("rematerialized_for_each_row_of",
                                    t.table()->alias);
      }
    }
  }

  return false;
}

/**
  Give error if we some tables are done with a full join.

  This is used by multi_table_update and multi_table_delete when running
  in safe mode.

  @param join		Join condition

  @retval
    0	ok
  @retval
    1	Error (full join used)
*/

bool error_if_full_join(JOIN *join) {
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  for (uint i = 0; i < join->primary_tables; i++) {
    JOIN_TAB *const tab = join->best_ref[i];
    THD *thd = join->thd;

    /*
      Safe update error isn't returned if:
      1) It is  an EXPLAIN statement OR
      2) Table is not the target.

      Append the first warning (if any) to the error message. Allows the user
      to understand why index access couldn't be chosen.
    */

    if (!thd->lex->is_explain() && tab->table()->pos_in_table_list->updating &&
        tab->type() == JT_ALL) {
      my_error(ER_UPDATE_WITHOUT_KEY_IN_SAFE_MODE, MYF(0),
               thd->get_stmt_da()->get_first_condition_message());
      return true;
    }
  }
  return false;
}

void JOIN_TAB::set_table(TABLE *t) {
  if (t != nullptr) t->reginfo.join_tab = this;
  m_qs->set_table(t);
}

void JOIN_TAB::init_join_cond_ref(TABLE_LIST *tl) {
  m_join_cond_ref = tl->join_cond_optim_ref();
}

/**
  Cleanup table of join operation.
*/

void JOIN_TAB::cleanup() {
  // Delete parts specific of JOIN_TAB:

  if (table()) table()->reginfo.join_tab = nullptr;

  // Delete shared parts:
  if (join()->qep_tab) {
    // deletion will be done by QEP_TAB
  } else
    qs_cleanup();
}

void QEP_TAB::cleanup() {
  // Delete parts specific of QEP_TAB:
  destroy(filesort);
  filesort = nullptr;
  if (quick_optim() != quick()) delete quick_optim();

  TABLE *const t = table();

  if (t != nullptr) {
    t->reginfo.qep_tab = nullptr;
    t->const_table = false;  // Note: Also done in TABLE::init()
  }

  // Delete shared parts:
  qs_cleanup();

  // Order of qs_cleanup() and this, matters:
  if (op_type == QEP_TAB::OT_MATERIALIZE ||
      op_type == QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE ||
      op_type == QEP_TAB::OT_AGGREGATE_INTO_TMP_TABLE ||
      op_type == QEP_TAB::OT_WINDOWING_FUNCTION) {
    if (t != nullptr)  // Check tmp table is not yet freed.
    {
      close_tmp_table(t);
      free_tmp_table(t);
    }
    destroy(tmp_table_param);
    tmp_table_param = nullptr;
  }
  if (table_ref != nullptr && table_ref->uses_materialization()) {
    assert(t == table_ref->table);
    t->merge_keys.clear_all();
    t->quick_keys.clear_all();
    t->covering_keys.clear_all();
    t->possible_quick_keys.clear_all();

    close_tmp_table(t);
  }
}

void QEP_shared_owner::qs_cleanup() {
  /* Skip non-existing derived tables/views result tables */
  if (table() &&
      (table()->s->tmp_table != INTERNAL_TMP_TABLE || table()->is_created())) {
    table()->set_keyread(false);
    table()->file->ha_index_or_rnd_end();
    free_io_cache(table());
    filesort_free_buffers(table(), true);
    TABLE_LIST *const table_ref = table()->pos_in_table_list;
    if (table_ref) {
      table_ref->derived_keys_ready = false;
      table_ref->derived_key_list.clear();
    }
  }
  delete quick();
}

uint QEP_TAB::sjm_query_block_id() const {
  assert(sj_is_materialize_strategy(get_sj_strategy()));
  for (uint i = 0; i < join()->primary_tables; ++i) {
    // Find the sj-mat tmp table whose sj nest contains us:
    Semijoin_mat_exec *const sjm = join()->qep_tab[i].sj_mat_exec();
    if (sjm && (uint)idx() >= sjm->inner_table_index &&
        (uint)idx() < sjm->inner_table_index + sjm->table_count)
      return sjm->sj_nest->nested_join->query_block_id;
  }
  assert(false);
  return 0;
}

/**
  Extend join_tab->cond by AND'ing add_cond to it

  @param add_cond    The condition to AND with the existing cond
                     for this JOIN_TAB

  @retval true   if there was a memory allocation error
  @retval false  otherwise
*/
bool QEP_shared_owner::and_with_condition(Item *add_cond) {
  Item *tmp = condition();
  if (and_conditions(&tmp, add_cond)) return true;
  set_condition(tmp);
  return false;
}

/**
  Partially cleanup JOIN after it has executed: close index or rnd read
  (table cursors), free quick selects.

    This function is called in the end of execution of a JOIN, before the used
    tables are unlocked and closed.

    For a join that is resolved using a temporary table, the first sweep is
    performed against actual tables and an intermediate result is inserted
    into the temprorary table.
    The last sweep is performed against the temporary table. Therefore,
    the base tables and associated buffers used to fill the temporary table
    are no longer needed, and this function is called to free them.

    For a join that is performed without a temporary table, this function
    is called after all rows are sent, but before EOF packet is sent.

    For a simple SELECT with no subqueries this function performs a full
    cleanup of the JOIN and calls mysql_unlock_read_tables to free used base
    tables.

    If a JOIN is executed for a subquery or if it has a subquery, we can't
    do the full cleanup and need to do a partial cleanup only.
    - If a JOIN is not the top level join, we must not unlock the tables
    because the outer select may not have been evaluated yet, and we
    can't unlock only selected tables of a query.
    - Additionally, if this JOIN corresponds to a correlated subquery, we
    should not free quick selects and join buffers because they will be
    needed for the next execution of the correlated subquery.
    - However, if this is a JOIN for a [sub]select, which is not
    a correlated subquery itself, but has subqueries, we can free it
    fully and also free JOINs of all its subqueries. The exception
    is a subquery in SELECT list, e.g:
    @code
    SELECT a, (select max(b) from t1) group by c
    @endcode
    This subquery will not be evaluated at first sweep and its value will
    not be inserted into the temporary table. Instead, it's evaluated
    when selecting from the temporary table. Therefore, it can't be freed
    here even though it's not correlated.

  @todo
    Unlock tables even if the join isn't top level select in the tree
*/

void JOIN::join_free() {
  Query_expression *tmp_query_expression;
  Query_block *sl;
  /*
    Optimization: if not EXPLAIN and we are done with the JOIN,
    free all tables.
  */
  bool full = (!query_block->uncacheable && !thd->lex->is_explain());
  bool can_unlock = full;
  DBUG_TRACE;

  cleanup();

  for (tmp_query_expression = query_block->first_inner_query_expression();
       tmp_query_expression;
       tmp_query_expression = tmp_query_expression->next_query_expression())
    for (sl = tmp_query_expression->first_query_block(); sl;
         sl = sl->next_query_block()) {
      Item_subselect *subselect = sl->master_query_expression()->item;
      bool full_local = full && (!subselect || subselect->is_evaluated());
      /*
        If this join is evaluated, we can partially clean it up and clean up
        all its underlying joins even if they are correlated, only query plan
        is left in case a user will run EXPLAIN FOR CONNECTION.
        If this join is not yet evaluated, we still must clean it up to
        close its table cursors -- it may never get evaluated, as in case of
        ... HAVING FALSE OR a IN (SELECT ...))
        but all table cursors must be closed before the unlock.
      */
      sl->cleanup_all_joins();
      /* Can't unlock if at least one JOIN is still needed */
      can_unlock = can_unlock && full_local;
    }

  /*
    We are not using tables anymore
    Unlock all tables. We may be in an INSERT .... SELECT statement.
  */
  if (can_unlock && lock && thd->lock && !thd->locked_tables_mode &&
      !(query_block->active_options() & SELECT_NO_UNLOCK) &&
      !query_block->subquery_in_having &&
      (query_block == (thd->lex->unit->fake_query_block
                           ? thd->lex->unit->fake_query_block
                           : thd->lex->query_block))) {
    /*
      TODO: unlock tables even if the join isn't top level select in the
      tree.
    */
    mysql_unlock_read_tables(thd, lock);  // Don't free join->lock
    DEBUG_SYNC(thd, "after_join_free_unlock");
    lock = nullptr;
  }
}

static void cleanup_table(TABLE *table) {
  if (table->is_created()) {
    table->file->ha_index_or_rnd_end();
  }
  free_io_cache(table);
  filesort_free_buffers(table, false);
}

/**
  Free resources of given join.

  @note
    With subquery this function definitely will be called several times,
    but even for simple query it can be called several times.
*/

void JOIN::cleanup() {
  DBUG_TRACE;

  assert(const_tables <= primary_tables && primary_tables <= tables);

  if (qep_tab || join_tab || best_ref) {
    for (uint i = 0; i < tables; i++) {
      QEP_TAB *qtab;
      TABLE *table;
      if (qep_tab) {
        assert(!join_tab);
        qtab = &qep_tab[i];
        table = qtab->table();
      } else {
        qtab = nullptr;
        table = (join_tab ? &join_tab[i] : best_ref[i])->table();
      }
      if (!table) continue;
      cleanup_table(table);
    }
  } else if (thd->lex->using_hypergraph_optimizer) {
    for (TABLE_LIST *tl = query_block->leaf_tables; tl; tl = tl->next_leaf) {
      cleanup_table(tl->table);
    }
    for (JOIN::TemporaryTableToCleanup cleanup : temp_tables) {
      cleanup_table(cleanup.table);
    }
  }

  /* Restore ref array to original state */
  set_ref_item_slice(REF_SLICE_SAVED_BASE);
}

/**
  Filter out ORDER BY items that are equal to constants in WHERE condition

  This function is a limited version of remove_const() for use
  with non-JOIN statements (i.e. single-table UPDATE and DELETE).

  @param order            Linked list of ORDER BY arguments.
  @param where            Where condition.

  @return pointer to new filtered ORDER list or NULL if whole list eliminated

  @note
    This function overwrites input order list.
*/

ORDER *simple_remove_const(ORDER *order, Item *where) {
  if (order == nullptr || where == nullptr) return order;

  ORDER *first = nullptr, *prev = nullptr;
  for (; order; order = order->next) {
    assert(!order->item[0]->has_aggregation());  // should never happen
    if (!check_field_is_const(where, order->item[0])) {
      if (first == nullptr) first = order;
      if (prev != nullptr) prev->next = order;
      prev = order;
    }
  }
  if (prev != nullptr) prev->next = nullptr;
  return first;
}

/**
  Check if equality can be used to remove sub-clause of GROUP BY/ORDER BY

  @param func   comparison operator (= or <=>)
  @param v      variable comparison operand (validated to be equal to
                                             ordering expression)
  @param c      other comparison operand (likely to be a constant)

  @returns true if equality determines uniqueness, false otherwise

    Checks if an equality predicate can be used to remove a GROUP BY/ORDER BY
    sub-clause when it is known to be true for exactly one distinct value
    (e.g. "expr" == "const").
    Arguments must be of the same type because e.g. "string_field" = "int_const"
     may match more than one distinct value from the column.
*/
static bool equality_determines_uniqueness(const Item_func_comparison *func,
                                           const Item *v, const Item *c) {
  /*
    - The "c" argument must be a constant.
    - The result type of both arguments must be the same.
      However, since a temporal type is also classified as a string type,
      we do not allow a temporal constant to be considered equal to a
      variable character string.
    - If both arguments are strings, the comparison operator must have the same
      collation as the ordering operation applied to the variable expression.
  */
  return c->const_for_execution() && v->result_type() == c->result_type() &&
         (v->result_type() != STRING_RESULT ||
          (!(is_string_type(v->data_type()) &&
             is_temporal_type(c->data_type())) &&
           func->compare_collation() == v->collation.collation));
}

/*
  Return true if i1 and i2 (if any) are equal items,
  or if i1 is a wrapper item around the f2 field.
*/

static bool equal(const Item *i1, const Item *i2, const Field *f2) {
  assert((i2 == nullptr) ^ (f2 == nullptr));

  if (i2 != nullptr)
    return i1->eq(i2, true);
  else if (i1->type() == Item::FIELD_ITEM)
    return f2->eq(down_cast<const Item_field *>(i1)->field);
  else
    return false;
}

/**
  Check if a field is equal to a constant value in a condition

  @param      cond        condition to search within
  @param      order_item  Item to find in condition (if order_field is NULL)
  @param      order_field Field to find in condition (if order_item is NULL)
  @param[out] const_item  Used in calculation with conjunctive predicates,
                          must be NULL in outer-most call.

  @returns true if the field is a constant value in condition, false otherwise
*/
bool check_field_is_const(Item *cond, const Item *order_item,
                          const Field *order_field, Item **const_item) {
  assert((order_item == nullptr) ^ (order_field == nullptr));

  Item *intermediate = nullptr;
  if (const_item == nullptr) const_item = &intermediate;

  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const c = down_cast<Item_cond *>(cond);
    bool and_level = c->functype() == Item_func::COND_AND_FUNC;
    List_iterator_fast<Item> li(*c->argument_list());
    Item *item;
    while ((item = li++)) {
      if (check_field_is_const(item, order_item, order_field, const_item)) {
        if (and_level) return true;
      } else if (!and_level)
        return false;
    }
    return !and_level;
  } else {
    if (cond->type() != Item::FUNC_ITEM) return false;
    Item_func *const func = down_cast<Item_func *>(cond);
    if (func->functype() != Item_func::EQUAL_FUNC &&
        func->functype() != Item_func::EQ_FUNC)
      return false;
    Item_func_comparison *comp = down_cast<Item_func_comparison *>(func);
    Item *left = comp->arguments()[0];
    Item *right = comp->arguments()[1];
    if (equal(left, order_item, order_field)) {
      if (equality_determines_uniqueness(comp, left, right)) {
        if (*const_item != nullptr) return right->eq(*const_item, true);
        *const_item = right;
        return true;
      }
    } else if (equal(right, order_item, order_field)) {
      if (equality_determines_uniqueness(comp, right, left)) {
        if (*const_item != nullptr) return left->eq(*const_item, true);
        *const_item = left;
        return true;
      }
    }
  }
  return false;
}

/**
  Update TMP_TABLE_PARAM with count of the different type of fields.

  This function counts the number of fields, functions and sum
  functions (items with type SUM_FUNC_ITEM) for use by
  create_tmp_table() and stores it in the Temp_table_param object. It
  also resets and calculates the allow_group_via_temp_table property, which may
  have to be reverted if this function is called after deciding to use ROLLUP
  (see JOIN::optimize_rollup()).

  @param query_block           Query_block of query
  @param param                Description of temp table
  @param fields               List of fields to count
  @param reset_with_sum_func  Whether to reset with_sum_func of func items
  @param save_sum_fields      Count in the way create_tmp_table() expects when
                              given the same parameter.
*/

void count_field_types(const Query_block *query_block, Temp_table_param *param,
                       const mem_root_deque<Item *> &fields,
                       bool reset_with_sum_func, bool save_sum_fields) {
  DBUG_TRACE;

  param->field_count = 0;
  param->sum_func_count = 0;
  param->func_count = 0;
  param->hidden_field_count = 0;
  param->outer_sum_func_count = 0;
  param->allow_group_via_temp_table = true;
  /*
    Loose index scan guarantees that all grouping is done and MIN/MAX
    functions are computed, so create_tmp_table() treats this as if
    save_sum_fields is set.
  */
  save_sum_fields |= param->precomputed_group_by;

  for (Item *field : fields) {
    Item *real = field->real_item();
    Item::Type real_type = real->type();

    if (real_type == Item::FIELD_ITEM)
      param->field_count++;
    else if (real_type == Item::SUM_FUNC_ITEM && !real->m_is_window_function) {
      if (!field->const_item()) {
        Item_sum *sum_item = down_cast<Item_sum *>(field->real_item());
        if (sum_item->aggr_query_block == query_block) {
          if (!sum_item->allow_group_via_temp_table)
            param->allow_group_via_temp_table = false;  // UDF SUM function
          param->sum_func_count++;

          for (uint i = 0; i < sum_item->argument_count();
               i++) {  // Add one column per argument
            if (sum_item->get_arg(i)->real_item()->type() == Item::FIELD_ITEM)
              param->field_count++;
            else
              param->func_count++;
          }
        }
        param->func_count++;  // A group aggregate function is a function!
      } else if (save_sum_fields) {
        /*
          Count the way create_tmp_table() does if asked to preserve
          Item_sum_* functions in fields list.

          Item field is an Item_sum_* or a reference to such an
          item. We need to distinguish between these two cases since
          they are treated differently by create_tmp_table().
        */
        if (field->type() == Item::SUM_FUNC_ITEM)  // An Item_sum_*
          param->field_count++;
        else  // A reference to an Item_sum_*
        {
          param->func_count++;
          param->sum_func_count++;
        }
      }
    } else if (real_type == Item::SUM_FUNC_ITEM) {
      assert(real->m_is_window_function);
      param->func_count++;

      Item_sum *window_item = down_cast<Item_sum *>(real);
      for (uint i = 0; i < window_item->argument_count(); i++) {
        if (window_item->get_arg(i)->real_item()->type() == Item::FIELD_ITEM)
          param->field_count++;
        else
          param->func_count++;
      }
    } else {
      param->func_count++;
      if (reset_with_sum_func) field->reset_aggregation();
      if (field->has_aggregation()) param->outer_sum_func_count++;
    }
  }
}

/**
  Return 1 if second is a subpart of first argument.

  If first parts has different direction, change it to second part
  (group is sorted like order)
*/

bool test_if_subpart(ORDER *a, ORDER *b) {
  ORDER *first = a;
  ORDER *second = b;
  for (; first && second; first = first->next, second = second->next) {
    if ((*first->item)->eq(*second->item, true))
      continue;
    else
      return false;
  }
  // If the second argument is not subpart of the first return false
  if (second) return false;
  // Else assign the direction of the second argument to the first
  else {
    for (; a && b; a = a->next, b = b->next) a->direction = b->direction;
    return true;
  }
}

/**
  calc how big buffer we need for comparing group entries.
*/

void calc_group_buffer(JOIN *join, ORDER *group) {
  DBUG_TRACE;
  uint key_length = 0, parts = 0, null_parts = 0;

  if (group) join->grouped = true;
  for (; group; group = group->next) {
    Item *group_item = *group->item;
    Field *field = group_item->get_tmp_table_field();
    if (field) {
      enum_field_types type;
      if ((type = field->type()) == MYSQL_TYPE_BLOB)
        key_length += MAX_BLOB_WIDTH;  // Can't be used as a key
      else if (type == MYSQL_TYPE_VARCHAR || type == MYSQL_TYPE_VAR_STRING)
        key_length += field->field_length + HA_KEY_BLOB_LENGTH;
      else if (type == MYSQL_TYPE_BIT) {
        /* Bit is usually stored as a longlong key for group fields */
        key_length += 8;  // Big enough
      } else
        key_length += field->pack_length();
    } else {
      switch (group_item->result_type()) {
        case REAL_RESULT:
          key_length += sizeof(double);
          break;
        case INT_RESULT:
          key_length += sizeof(longlong);
          break;
        case DECIMAL_RESULT:
          key_length += my_decimal_get_binary_size(
              group_item->max_length - (group_item->decimals ? 1 : 0),
              group_item->decimals);
          break;
        case STRING_RESULT: {
          /*
            As items represented as DATE/TIME fields in the group buffer
            have STRING_RESULT result type, we increase the length
            by 8 as maximum pack length of such fields.
          */
          if (group_item->is_temporal()) {
            key_length += 8;
          } else if (group_item->data_type() == MYSQL_TYPE_BLOB)
            key_length += MAX_BLOB_WIDTH;  // Can't be used as a key
          else {
            /*
              Group strings are taken as varstrings and require an length field.
              A field is not yet created by create_tmp_field()
              and the sizes should match up.
            */
            key_length += group_item->max_length + HA_KEY_BLOB_LENGTH;
          }
          break;
        }
        default:
          /* This case should never be choosen */
          assert(0);
          my_error(ER_OUT_OF_RESOURCES, MYF(ME_FATALERROR));
      }
    }
    parts++;
    if (group_item->is_nullable()) null_parts++;
  }
  join->tmp_table_param.group_length = key_length + null_parts;
  join->tmp_table_param.group_parts = parts;
  join->tmp_table_param.group_null_parts = null_parts;
}

/**
  Make an array of pointers to sum_functions to speed up
  sum_func calculation.

  @retval
    0	ok
  @retval
    1	Error
*/

bool JOIN::alloc_func_list() {
  uint func_count, group_parts;
  DBUG_TRACE;

  func_count = tmp_table_param.sum_func_count;
  /*
    If we are using rollup, we need a copy of the summary functions for
    each level
  */
  if (rollup_state != RollupState::NONE) func_count *= (send_group_parts + 1);

  group_parts = send_group_parts;
  /*
    If distinct, reserve memory for possible
    disctinct->group_by optimization
  */
  if (select_distinct) {
    group_parts += CountVisibleFields(*fields);
    /*
      If the ORDER clause is specified then it's possible that
      it also will be optimized, so reserve space for it too
    */
    if (!order.empty()) {
      ORDER *ord;
      for (ord = order.order; ord; ord = ord->next) group_parts++;
    }
  }

  /* This must use calloc() as rollup_make_fields depends on this */
  sum_funcs =
      (Item_sum **)thd->mem_calloc(sizeof(Item_sum **) * (func_count + 1) +
                                   sizeof(Item_sum ***) * (group_parts + 1));
  return sum_funcs == nullptr;
}

/**
  Initialize 'sum_funcs' array with all Item_sum objects.

  @param fields            All items
  @param before_group_by   Set to 1 if this is called before GROUP BY handling
  @param recompute         Set to true if sum_funcs must be recomputed

  @retval
    0  ok
  @retval
    1  error
*/

bool JOIN::make_sum_func_list(const mem_root_deque<Item *> &fields,
                              bool before_group_by, bool recompute) {
  DBUG_TRACE;

  if (*sum_funcs && !recompute)
    return false; /* We have already initialized sum_funcs. */

  Item_sum **func = sum_funcs;
  for (Item *item : fields) {
    if (item->type() == Item::SUM_FUNC_ITEM && !item->const_item() &&
        down_cast<Item_sum *>(item)->aggr_query_block == query_block) {
      assert(!item->m_is_window_function);
      *func++ = down_cast<Item_sum *>(item);
    }
  }
  if (before_group_by && rollup_state == RollupState::INITED) {
    rollup_state = RollupState::READY;
  } else if (rollup_state == RollupState::READY)
    return false;   // Don't put end marker
  *func = nullptr;  // End marker
  return false;
}

/**
  Free joins of subselect of this select.

  @param thd      thread handle
  @param select   pointer to Query_block which subselects joins we will free

  @todo when the final use of this function (from SET statements) is removed,
  this function can be deleted.
*/

void free_underlaid_joins(THD *thd, Query_block *select) {
  for (Query_expression *query_expression =
           select->first_inner_query_expression();
       query_expression;
       query_expression = query_expression->next_query_expression())
    query_expression->cleanup(thd, false);
}

/**
  Change the Query_result object of the query block.

  If old_result is not used, forward the call to the current
  Query_result in case it is a wrapper around old_result.

  Call prepare() on the new Query_result if we decide to use it.

  @param thd        Thread handle
  @param new_result New Query_result object
  @param old_result Old Query_result object (NULL to force change)

  @retval false Success
  @retval true  Error
*/

bool Query_block::change_query_result(THD *thd,
                                      Query_result_interceptor *new_result,
                                      Query_result_interceptor *old_result) {
  DBUG_TRACE;
  if (old_result == nullptr || query_result() == old_result) {
    set_query_result(new_result);
    if (query_result()->prepare(thd, fields, master_query_expression()))
      return true; /* purecov: inspected */
    return false;
  } else {
    const bool ret = query_result()->change_query_result(thd, new_result);
    return ret;
  }
}

/**
  Add having condition as a filter condition, which is applied when reading
  from the temp table.

  @param    curr_tmp_table  Table number to which having conds are added.
  @returns  false if success, true if error.
*/

bool JOIN::add_having_as_tmp_table_cond(uint curr_tmp_table) {
  having_cond->update_used_tables();
  QEP_TAB *const curr_table = &qep_tab[curr_tmp_table];
  table_map used_tables;
  Opt_trace_context *const trace = &thd->opt_trace;

  DBUG_TRACE;

  if (curr_table->table_ref)
    used_tables = curr_table->table_ref->map();
  else {
    /*
      Pushing parts of HAVING to an internal temporary table.
      Fields in HAVING condition may have been replaced with fields in an
      internal temporary table. This table has map=1.
    */
    assert(having_cond->has_subquery() ||
           !(having_cond->used_tables() & ~(1 | PSEUDO_TABLE_BITS)));
    used_tables = 1;
  }
  // Condition may contain outer references, const and non-deterministic exprs:
  used_tables |= PSEUDO_TABLE_BITS;

  /*
    All conditions which can be applied after reading from used_tables are
    added as filter conditions of curr_tmp_table. If condition's used_tables is
    not read yet for example subquery in having, then it will be kept as it is
    in original having_cond of join.
    If ROLLUP, having condition needs to be tested after writing rollup data.
    So do not move the having condition.
  */
  Item *sort_table_cond =
      (rollup_state == RollupState::NONE)
          ? make_cond_for_table(thd, having_cond, used_tables, table_map{0},
                                false)
          : nullptr;
  if (sort_table_cond) {
    if (!curr_table->condition())
      curr_table->set_condition(sort_table_cond);
    else {
      curr_table->set_condition(
          new Item_cond_and(curr_table->condition(), sort_table_cond));
      if (curr_table->condition()->fix_fields(thd, nullptr)) return true;
    }
    curr_table->condition()->apply_is_true();
    DBUG_EXECUTE("where", print_where(thd, curr_table->condition(),
                                      "select and having", QT_ORDINARY););

    having_cond = make_cond_for_table(thd, having_cond, ~table_map{0},
                                      ~used_tables, false);
    DBUG_EXECUTE("where", print_where(thd, having_cond, "having after sort",
                                      QT_ORDINARY););

    Opt_trace_object trace_wrapper(trace);
    Opt_trace_object(trace, "sort_using_internal_table")
        .add("condition_for_sort", sort_table_cond)
        .add("having_after_sort", having_cond);
  }

  return false;
}

/**
  Init tmp tables usage info.

  @details
  This function finalizes execution plan by taking following actions:
    .) tmp tables are created, but not instantiated (this is done during
       execution). QEP_TABs dedicated to tmp tables are filled appropriately.
       see JOIN::create_intermediate_table.
    .) prepare fields lists (fields, all_fields, ref_item_array slices) for
       each required stage of execution. These fields lists are set for
       tmp tables' tabs and for the tab of last table in the join.
    .) fill info for sorting/grouping/dups removal is prepared and saved to
       appropriate tabs. Here is an example:
        SELECT * from t1,t2 WHERE ... GROUP BY t1.f1 ORDER BY t2.f2, t1.f2
        and lets assume that the table order in the plan is t1,t2.
       In this case optimizer will sort for group only the first table as the
       second one isn't mentioned in GROUP BY. The result will be materialized
       in tmp table.  As filesort can't sort join optimizer will sort tmp table
       also. The first sorting (for group) is called simple as is doesn't
       require tmp table.  The Filesort object for it is created here - in
       JOIN::create_intermediate_table.  Filesort for the second case is
       created here, in JOIN::make_tmp_tables_info.

  @note
  This function may change tmp_table_param.precomputed_group_by. This
  affects how create_tmp_table() treats aggregation functions, so
  count_field_types() must be called again to make sure this is taken
  into consideration.

  @returns
  false - Ok
  true  - Error
*/

bool JOIN::make_tmp_tables_info() {
  assert(!join_tab);
  mem_root_deque<Item *> *curr_fields = fields;
  bool materialize_join = false;
  uint curr_tmp_table = const_tables;
  TABLE *exec_tmp_table = nullptr;

  /*
    If the plan is constant, we will not do window tmp table processing
    cf. special code path in do_query_block.
  */
  m_windowing_steps = m_windows.elements > 0 && !plan_is_const() &&
                      !implicit_grouping && !group_optimized_away;
  const bool may_trace =  // just to avoid an empty trace block
      need_tmp_before_win || implicit_grouping || m_windowing_steps ||
      !group_list.empty() || !order.empty();

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_disable_I_S trace_disabled(trace, !may_trace);
  Opt_trace_object wrapper(trace);
  Opt_trace_array trace_tmp(trace, "considering_tmp_tables");

  DBUG_TRACE;

  /*
    In this function, we may change having_cond into a condition on a
    temporary sort/group table, so we have to assign having_for_explain now:
  */
  having_for_explain = having_cond;

  const bool has_group_by = this->grouped;

  /*
    The loose index scan access method guarantees that all grouping or
    duplicate row elimination (for distinct) is already performed
    during data retrieval, and that all MIN/MAX functions are already
    computed for each group. Thus all MIN/MAX functions should be
    treated as regular functions, and there is no need to perform
    grouping in the main execution loop.
    Notice that currently loose index scan is applicable only for
    single table queries, thus it is sufficient to test only the first
    join_tab element of the plan for its access method.
  */
  if (qep_tab && qep_tab[0].quick() &&
      qep_tab[0].quick()->is_loose_index_scan())
    tmp_table_param.precomputed_group_by =
        !qep_tab[0].quick()->is_agg_loose_index_scan();

  uint last_slice_before_windowing = REF_SLICE_ACTIVE;

  /*
    Create the first temporary table if distinct elimination is requested or
    if the sort is too complicated to be evaluated as a filesort.
  */
  if (need_tmp_before_win) {
    curr_tmp_table = primary_tables;
    Opt_trace_object trace_this_outer(trace);
    trace_this_outer.add("adding_tmp_table_in_plan_at_position",
                         curr_tmp_table);
    tmp_tables++;

    /*
      Make a copy of the base slice in the save slice.
      This is needed because later steps will overwrite the base slice with
      another slice (1-3).
      After this slice has been used, overwrite the base slice again with
      the copy in the save slice.
    */
    if (alloc_ref_item_slice(thd, REF_SLICE_SAVED_BASE)) return true;

    copy_ref_item_slice(REF_SLICE_SAVED_BASE, REF_SLICE_ACTIVE);
    current_ref_item_slice = REF_SLICE_SAVED_BASE;

    /*
      Create temporary table for use in a single execution.
      (Will be reused if this is a subquery that is executed several times
       for one execution of the statement)
      Don't use tmp table grouping for json aggregate funcs as it's
      very ineffective.
    */
    ORDER_with_src tmp_group;
    if (!simple_group && !(test_flags & TEST_NO_KEY_GROUP) && !with_json_agg)
      tmp_group = group_list;

    tmp_table_param.hidden_field_count = CountHiddenFields(*fields);

    if (create_intermediate_table(&qep_tab[curr_tmp_table], *fields, tmp_group,
                                  !group_list.empty() && simple_group))
      return true;
    exec_tmp_table = qep_tab[curr_tmp_table].table();

    if (exec_tmp_table->s->is_distinct) optimize_distinct();

    /*
      If there is no sorting or grouping, 'use_order'
      index result should not have been requested.
      Exception: LooseScan strategy for semijoin requires
      sorted access even if final result is not to be sorted.
    */
    assert(
        !(m_ordered_index_usage == ORDERED_INDEX_VOID && !plan_is_const() &&
          qep_tab[const_tables].position()->sj_strategy != SJ_OPT_LOOSE_SCAN &&
          qep_tab[const_tables].use_order()));

    /*
      Allocate a slice of ref items that describe the items to be copied
      from the first temporary table.
    */
    if (alloc_ref_item_slice(thd, REF_SLICE_TMP1)) return true;

    // Change sum_fields reference to calculated fields in tmp_table
    if (streaming_aggregation || qep_tab[curr_tmp_table].table()->group ||
        tmp_table_param.precomputed_group_by) {
      if (change_to_use_tmp_fields(fields, thd, ref_items[REF_SLICE_TMP1],
                                   &tmp_fields[REF_SLICE_TMP1],
                                   query_block->m_added_non_hidden_fields))
        return true;
    } else {
      if (change_to_use_tmp_fields_except_sums(
              fields, thd, query_block, ref_items[REF_SLICE_TMP1],
              &tmp_fields[REF_SLICE_TMP1],
              query_block->m_added_non_hidden_fields))
        return true;
    }
    curr_fields = &tmp_fields[REF_SLICE_TMP1];
    // Need to set them now for correct group_fields setup, reset at the end.
    set_ref_item_slice(REF_SLICE_TMP1);
    qep_tab[curr_tmp_table].ref_item_slice = REF_SLICE_TMP1;
    setup_tmptable_write_func(&qep_tab[curr_tmp_table], &trace_this_outer);
    last_slice_before_windowing = REF_SLICE_TMP1;

    /*
      If having is not handled here, it will be checked before the row is sent
      to the client.
    */
    if (having_cond &&
        (streaming_aggregation ||
         (exec_tmp_table->s->is_distinct && group_list.empty()))) {
      /*
        If there is no select distinct or rollup, then move the having to table
        conds of tmp table.
        NOTE : We cannot apply having after distinct. If columns of having are
               not part of select distinct, then distinct may remove rows
               which can satisfy having.

        As this condition will read the tmp table, it is appropriate that
        REF_SLICE_TMP1 is in effect when we create it below.
      */
      if ((!select_distinct && rollup_state == RollupState::NONE) &&
          add_having_as_tmp_table_cond(curr_tmp_table))
        return true;

      /*
        Having condition which we are not able to add as tmp table conds are
        kept as before. And, this will be applied before storing the rows in
        tmp table.
      */
      qep_tab[curr_tmp_table].having = having_cond;
      having_cond = nullptr;  // Already done
    }

    tmp_table_param.func_count = 0;

    if (streaming_aggregation || qep_tab[curr_tmp_table].table()->group) {
      tmp_table_param.field_count += tmp_table_param.sum_func_count;
      tmp_table_param.sum_func_count = 0;
    }

    if (exec_tmp_table->group) {  // Already grouped
                                  /*
                                    Check if group by has to respect ordering. If true, move group by to
                                    order by.
                                  */
      if (order.empty() && !skip_sort_order) {
        for (ORDER *group = group_list.order; group; group = group->next) {
          if (group->direction != ORDER_NOT_RELEVANT) {
            order = group_list; /* order by group */
            break;
          }
        }
      }
      group_list.clean();
    }
    /*
      If we have different sort & group then we must sort the data by group
      and copy it to a second temporary table.
      This code is also used if we are using distinct something
      we haven't been able to store in the temporary table yet
      like SEC_TO_TIME(SUM(...)) or when distinct is used with rollup.
    */

    if ((!group_list.empty() &&
         (!test_if_subpart(group_list.order, order.order) || select_distinct ||
          m_windowing_steps || rollup_state != RollupState::NONE)) ||
        (select_distinct && (tmp_table_param.using_outer_summary_function ||
                             rollup_state != RollupState::NONE))) {
      DBUG_PRINT("info", ("Creating group table"));

      calc_group_buffer(this, group_list.order);
      count_field_types(query_block, &tmp_table_param,
                        tmp_fields[REF_SLICE_TMP1],
                        select_distinct && group_list.empty(), false);
      tmp_table_param.hidden_field_count =
          CountHiddenFields(tmp_fields[REF_SLICE_TMP1]);
      streaming_aggregation = false;
      if (!exec_tmp_table->group && !exec_tmp_table->s->is_distinct) {
        // 1st tmp table were materializing join result
        materialize_join = true;
        explain_flags.set(ESC_BUFFER_RESULT, ESP_USING_TMPTABLE);
      }
      curr_tmp_table++;
      tmp_tables++;
      Opt_trace_object trace_this_tbl(trace);
      trace_this_tbl.add("adding_tmp_table_in_plan_at_position", curr_tmp_table)
          .add_alnum("cause", "sorting_to_make_groups");

      /* group data to new table */
      /*
        If the access method is loose index scan then all MIN/MAX
        functions are precomputed, and should be treated as regular
        functions. See extended comment above.
      */
      if (qep_tab[0].quick() && qep_tab[0].quick()->is_loose_index_scan())
        tmp_table_param.precomputed_group_by = true;

      ORDER_with_src dummy;  // TODO can use table->group here also

      if (create_intermediate_table(&qep_tab[curr_tmp_table], *curr_fields,
                                    dummy, true))
        return true;

      if (!group_list.empty()) {
        explain_flags.set(group_list.src, ESP_USING_TMPTABLE);
        if (!plan_is_const())  // No need to sort a single row
        {
          if (add_sorting_to_table(curr_tmp_table - 1, &group_list,
                                   /*force_stable_sort=*/false,
                                   /*sort_before_group=*/true))
            return true;
        }

        if (make_group_fields(this, this)) return true;
      }

      // Setup sum funcs only when necessary, otherwise we might break info
      // for the first table
      if (!group_list.empty() || tmp_table_param.sum_func_count) {
        if (make_sum_func_list(*curr_fields, true, true)) return true;
        const bool need_distinct =
            !(qep_tab[0].quick() &&
              qep_tab[0].quick()->is_agg_loose_index_scan());
        if (prepare_sum_aggregators(sum_funcs, need_distinct)) return true;
        group_list.clean();
        if (setup_sum_funcs(thd, sum_funcs)) return true;
      }

      /*
        Allocate a slice of ref items that describe the items to be copied
        from the second temporary table.
      */
      if (alloc_ref_item_slice(thd, REF_SLICE_TMP2)) return true;

      // No sum funcs anymore
      if (change_to_use_tmp_fields(&tmp_fields[REF_SLICE_TMP1], thd,
                                   ref_items[REF_SLICE_TMP2],
                                   &tmp_fields[REF_SLICE_TMP2],
                                   query_block->m_added_non_hidden_fields))
        return true;

      curr_fields = &tmp_fields[REF_SLICE_TMP2];
      set_ref_item_slice(REF_SLICE_TMP2);
      qep_tab[curr_tmp_table].ref_item_slice = REF_SLICE_TMP2;
      setup_tmptable_write_func(&qep_tab[curr_tmp_table], &trace_this_tbl);
      last_slice_before_windowing = REF_SLICE_TMP2;
    }
    if (qep_tab[curr_tmp_table].table()->s->is_distinct)
      select_distinct = false; /* Each row is unique */

    if (select_distinct && group_list.empty() && !m_windowing_steps) {
      if (having_cond) {
        qep_tab[curr_tmp_table].having = having_cond;
        having_cond->update_used_tables();
        having_cond = nullptr;
      }
      qep_tab[curr_tmp_table].needs_duplicate_removal = true;
      trace_this_outer.add("reading_from_table_eliminates_duplicates", true);
      explain_flags.set(ESC_DISTINCT, ESP_DUPS_REMOVAL);
      select_distinct = false;
    }
    /* Clean tmp_table_param for the next tmp table. */
    tmp_table_param.field_count = tmp_table_param.sum_func_count =
        tmp_table_param.func_count = 0;

    tmp_table_param.cleanup();
    streaming_aggregation = false;

    if (!group_optimized_away) {
      grouped = false;
    } else {
      /*
        If grouping has been optimized away, a temporary table is
        normally not needed unless we're explicitly requested to create
        one (e.g. due to a SQL_BUFFER_RESULT hint or INSERT ... SELECT or
        there is a windowing function that needs sorting).

        In this case (grouping was optimized away), temp_table was
        created without a grouping expression and JOIN::exec() will not
        perform the necessary grouping (by the use of end_send_group()
        or end_write_group()) if JOIN::group is set to false.
      */
      /*
         The temporary table was explicitly requested or there is a window
         function which needs sorting (check need_tmp_before_win in
         JOIN::optimize).
      */
      assert(query_block->active_options() & OPTION_BUFFER_RESULT ||
             m_windowing_steps);
      // the temporary table does not have a grouping expression
      assert(!qep_tab[curr_tmp_table].table()->group);
    }
    calc_group_buffer(this, group_list.order);
    count_field_types(query_block, &tmp_table_param, *curr_fields, false,
                      false);
  }

  /*
    Set up structures for a temporary table but do not actually create
    the temporary table if one of these conditions are true:
    - The query is implicitly grouped.
    - The query is explicitly grouped and
        + implemented as a simple grouping, or
        + LIMIT 1 is specified, or
        + ROLLUP is specified, or
        + <some unknown condition>.
  */

  if ((grouped || implicit_grouping) && !m_windowing_steps) {
    if (make_group_fields(this, this)) return true;

    if (make_sum_func_list(*curr_fields, true, true)) return true;
    const bool need_distinct = !(qep_tab && qep_tab[0].quick() &&
                                 qep_tab[0].quick()->is_agg_loose_index_scan());
    if (prepare_sum_aggregators(sum_funcs, need_distinct)) return true;
    if (setup_sum_funcs(thd, sum_funcs) || thd->is_fatal_error()) return true;
  }

  if (qep_tab && (!group_list.empty() ||
                  (!order.empty() && !m_windowing_steps /* [1] */))) {
    /*
      [1] above: too early to do query ORDER BY if we have windowing; must
      wait till after window processing.
    */
    ASSERT_BEST_REF_IN_JOIN_ORDER(this);
    DBUG_PRINT("info", ("Sorting for send_result_set_metadata"));
    /*
      If we have already done the group, add HAVING to sorted table except
      when rollup is present
    */
    if (having_cond && group_list.empty() && !streaming_aggregation &&
        rollup_state == RollupState::NONE) {
      if (add_having_as_tmp_table_cond(curr_tmp_table)) return true;
    }

    if (grouped)
      m_select_limit = HA_POS_ERROR;
    else if (!need_tmp_before_win) {
      /*
        We can abort sorting after thd->select_limit rows if there are no
        filter conditions for any tables after the sorted one.
        Filter conditions come in several forms:
         1. as a condition item attached to the join_tab, or
         2. as a keyuse attached to the join_tab (ref access).
      */
      for (uint i = const_tables + 1; i < primary_tables; i++) {
        QEP_TAB *const tab = qep_tab + i;
        if (tab->condition() ||  // 1
            (best_ref[tab->idx()]->keyuse() &&
             tab->first_inner() == NO_PLAN_IDX))  // 2
        {
          /* We have to sort all rows */
          m_select_limit = HA_POS_ERROR;
          break;
        }
      }
    }
    /*
      Here we add sorting stage for ORDER BY/GROUP BY clause, if the
      optimiser chose FILESORT to be faster than INDEX SCAN or there is
      no suitable index present.
      OPTION_FOUND_ROWS supersedes LIMIT and is taken into account.
    */
    DBUG_PRINT("info", ("Sorting for order by/group by"));
    ORDER_with_src order_arg = group_list.empty() ? order : group_list;
    if (qep_tab &&
        m_ordered_index_usage != (group_list.empty()
                                      ? ORDERED_INDEX_ORDER_BY
                                      : ORDERED_INDEX_GROUP_BY) &&
        // Windowing will change order, so it's too early to sort here
        !m_windowing_steps) {
      // Sort either first non-const table or the last tmp table
      QEP_TAB *const sort_tab = &qep_tab[curr_tmp_table];
      if (need_tmp_before_win && !materialize_join && !exec_tmp_table->group)
        explain_flags.set(order_arg.src, ESP_USING_TMPTABLE);

      if (add_sorting_to_table(curr_tmp_table, &order_arg,
                               /*force_stable_sort=*/false,
                               /*sort_before_group=*/false))
        return true;
      /*
        filesort_limit:	 Return only this many rows from filesort().
        We can use select_limit_cnt only if we have no group_by and 1 table.
        This allows us to use Bounded_queue for queries like:
          "select * from t1 order by b desc limit 1;"
        m_select_limit == HA_POS_ERROR (we need a full table scan)
        query_expression->select_limit_cnt == 1 (we only need one row in the
        result set)
      */
      if (sort_tab->filesort)
        sort_tab->filesort->limit =
            (has_group_by || (primary_tables > curr_tmp_table + 1) ||
             calc_found_rows)
                ? m_select_limit
                : query_expression()->select_limit_cnt;
    }
  }

  if (qep_tab && m_windowing_steps) {
    for (uint wno = 0; wno < m_windows.elements; wno++) {
      tmp_table_param.m_window = m_windows[wno];

      if (!tmp_tables) {
        curr_tmp_table = primary_tables;
        tmp_tables++;

        if (ref_items[REF_SLICE_SAVED_BASE].is_null()) {
          /*
           Make a copy of the base slice in the save slice.
           This is needed because later steps will overwrite the base slice with
           another slice (1-3 or window slice).
           After this slice has been used, overwrite the base slice again with
           the copy in the save slice.
           */
          if (alloc_ref_item_slice(thd, REF_SLICE_SAVED_BASE)) return true;

          copy_ref_item_slice(REF_SLICE_SAVED_BASE, REF_SLICE_ACTIVE);
          current_ref_item_slice = REF_SLICE_SAVED_BASE;
        }
      } else {
        curr_tmp_table++;
        tmp_tables++;
      }

      ORDER_with_src dummy;

      if (last_slice_before_windowing == REF_SLICE_ACTIVE) {
        tmp_table_param.hidden_field_count = CountHiddenFields(*fields);
      } else {
        assert(tmp_tables >= 1 &&
               last_slice_before_windowing > REF_SLICE_ACTIVE);

        tmp_table_param.hidden_field_count =
            CountHiddenFields(tmp_fields[last_slice_before_windowing]);
      }

      /*
        Allocate a slice of ref items that describe the items to be copied
        from the next temporary table.
      */
      const uint widx = REF_SLICE_WIN_1 + wno;
      const int fbidx = widx + m_windows.elements;  // use far area
      m_windows[wno]->set_needs_restore_input_row(
          wno == 0 && qep_tab[primary_tables - 1].type() == JT_EQ_REF);

      if (m_windows[wno]->needs_buffering()) {
        /*
          Create the window frame buffer tmp table.  We create a
          temporary table with same contents as the output tmp table
          in the windowing pipeline (columns defined by
          curr_all_fields), but used for intermediate storage, saving
          the window's frame buffer now that we know the window needs
          buffering.
        */
        Temp_table_param *par =
            new (thd->mem_root) Temp_table_param(tmp_table_param);
        par->m_window_frame_buffer = true;
        TABLE *table =
            create_tmp_table(thd, par, *curr_fields, nullptr, false, false,
                             query_block->active_options(), HA_POS_ERROR, "");
        if (table == nullptr) return true;

        if (alloc_ref_item_slice(thd, fbidx)) return true;

        if (change_to_use_tmp_fields(curr_fields, thd, ref_items[fbidx],
                                     &tmp_fields[fbidx],
                                     query_block->m_added_non_hidden_fields))
          return true;

        m_windows[wno]->set_frame_buffer_param(par);
        m_windows[wno]->set_frame_buffer(table);
      }

      Opt_trace_object trace_this_tbl(trace);
      trace_this_tbl.add("adding_tmp_table_in_plan_at_position", curr_tmp_table)
          .add_alnum("cause", "output_for_window_functions")
          .add("with_buffer", m_windows[wno]->needs_buffering());
      QEP_TAB *tab = &qep_tab[curr_tmp_table];
      if (create_intermediate_table(tab, *curr_fields, dummy, false))
        return true;

      m_windows[wno]->set_outtable_param(tab->tmp_table_param);

      if (m_windows[wno]->make_special_rows_cache(thd, tab->table()))
        return true;

      if (alloc_ref_item_slice(thd, widx)) return true;

      if (change_to_use_tmp_fields(
              (last_slice_before_windowing == REF_SLICE_ACTIVE
                   ? fields
                   : &tmp_fields[last_slice_before_windowing]),
              thd, ref_items[widx], &tmp_fields[widx],
              query_block->m_added_non_hidden_fields))
        return true;

      curr_fields = &tmp_fields[widx];
      set_ref_item_slice(widx);
      tab->ref_item_slice = widx;
      setup_tmptable_write_func(tab, &trace_this_tbl);

      ORDER_with_src w_partition(m_windows[wno]->sorting_order(),
                                 ESC_WINDOWING);

      if (w_partition.order != nullptr) {
        Opt_trace_object trace_pre_sort(trace, "adding_sort_to_previous_table");
        if (add_sorting_to_table(curr_tmp_table - 1, &w_partition,
                                 /*force_stable_sort=*/true,
                                 /*sort_before_group=*/false))
          return true;
      }

      if (m_windows[wno]->is_last()) {
        if (!order.empty() && m_ordered_index_usage != ORDERED_INDEX_ORDER_BY) {
          if (add_sorting_to_table(curr_tmp_table, &order,
                                   /*force_stable_sort=*/false,
                                   /*sort_before_group=*/false))
            return true;
        }
        if (!tab->filesort && !tab->table()->s->keys &&
            (!(query_block->active_options() & OPTION_BUFFER_RESULT) ||
             need_tmp_before_win || wno >= 1)) {
          /*
            Last tmp table of execution; no sort, no duplicate elimination, no
            buffering imposed by user (or it has already been implemented by
            a previous tmp table): hence any row needn't be written to
            tmp table's storage; send it out to query's result instead:
          */
          tab->tmp_table_param->m_window_short_circuit = true;
        }
      }

      if (having_cond != nullptr) {
        tab->having = having_cond;
        having_cond = nullptr;
      }

      last_slice_before_windowing = widx;
    }
  }

  {
    // In the case of rollup (only): After the base slice list was made, we may
    // have modified the field list to add rollup group items and sum switchers.
    // Since there may be HAVING filters with refs that refer to the base slice,
    // we need to refresh that slice (and its copy, REF_SLICE_SAVED_BASE) so
    // that it includes the updated items.
    //
    // Note that we do this after we've made the TMP1 and TMP2 slices, since
    // there's a lot of logic that looks through the GROUP BY list, which refers
    // to the base slice and expects _not_ to find rollup items there.
    unsigned num_hidden_fields = CountHiddenFields(*fields);
    const size_t num_select_elements = fields->size() - num_hidden_fields;
    const size_t orig_num_select_elements =
        num_select_elements - query_block->m_added_non_hidden_fields;

    for (unsigned i = 0; i < fields->size(); ++i) {
      Item *item = (*fields)[i];
      size_t pos;
      // See change_to_use_tmp_fields_except_sums for an explanation of how
      // the visible fields, hidden fields and additonal fields added by
      // transformations are organized in fields and ref_item_array.
      if (i < num_hidden_fields) {
        pos = fields->size() - i - 1 - query_block->m_added_non_hidden_fields;
      } else {
        pos = i - num_hidden_fields;
        if (pos >= orig_num_select_elements) pos += num_hidden_fields;
      }
      query_block->base_ref_items[pos] = item;
      if (!ref_items[REF_SLICE_SAVED_BASE].is_null()) {
        ref_items[REF_SLICE_SAVED_BASE][pos] = item;
      }
    }
  }

  fields = curr_fields;
  // Reset before execution
  set_ref_item_slice(REF_SLICE_SAVED_BASE);
  if (qep_tab) {
    qep_tab[primary_tables + tmp_tables].op_type = get_end_select_func();
  }
  grouped = has_group_by;

  unplug_join_tabs();

  /*
    Tmp tables are a layer between the nested loop and the derived table's
    result, WITH RECURSIVE cannot work with them. This should not happen, as a
    recursive query cannot have clauses which use a tmp table (GROUP BY,
    etc).
  */
  assert(!query_block->is_recursive() || !tmp_tables);
  return false;
}

void JOIN::unplug_join_tabs() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  /*
    During execution we will need to access QEP_TABs by map.
    map2table points to JOIN_TABs which are to be trashed a few lines down; so
    we won't use map2table, but build a similar map2qep_tab; no need to
    allocate new space for this array, we can reuse that of map2table.
  */
  static_assert(sizeof(QEP_TAB *) == sizeof(JOIN_TAB *), "");
  void *storage = reinterpret_cast<void *>(map2table);
  map2qep_tab = reinterpret_cast<QEP_TAB **>(storage);
  for (uint i = 0; i < tables; ++i)
    if (best_ref[i]->table_ref)
      map2qep_tab[best_ref[i]->table_ref->tableno()] = &qep_tab[i];

  map2table = nullptr;

  for (uint i = 0; i < tables; ++i) best_ref[i]->cleanup();

  best_ref = nullptr;
}

/**
  @brief Add Filesort object to the given table to sort if with filesort

  @param idx        JOIN_TAB's position in the qep_tab array. The
                    created Filesort object gets attached to this.

  @param sort_order List of expressions to sort the table by
  @param force_stable_sort
                    If true, use stable sort, that is the sort will
                    keep the reative order of equivalent elements.
                    Needed for windowing semantics.
  @param sort_before_group
                    If true, this sort happens before grouping is done
                    (potentially as a step of grouping itself),
                    so any wrapped rollup group items should be
                    unwrapped.

  @note This function moves tab->select, if any, to filesort->select

  @return false on success, true on OOM
*/

bool JOIN::add_sorting_to_table(uint idx, ORDER_with_src *sort_order,
                                bool force_stable_sort,
                                bool sort_before_group) {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  assert(!query_block->is_recursive());
  const enum join_type jt = qep_tab[idx].type();
  if (jt == JT_CONST || jt == JT_EQ_REF)
    return false;  // 1 single row: is already sorted

  // Weedout needs an underlying table to store refs from (it deduplicates
  // by row ID), so if this table is part of a weedout operation, we need
  // to force sorting by row IDs -- sorting rows with addon fields returns
  // rows that have no reference to the underlying table object.
  bool force_sort_position = false;
  for (plan_idx i = 0; i <= static_cast<plan_idx>(idx); ++i) {
    if (!qep_tab[i].starts_weedout()) {
      continue;
    }

    plan_idx weedout_end = NO_PLAN_IDX;  // Exclusive.
    for (uint j = i; j < primary_tables; ++j) {
      if (qep_tab[j].check_weed_out_table == qep_tab[i].flush_weedout_table) {
        weedout_end = j + 1;
        break;
      }
    }
    if (weedout_end != NO_PLAN_IDX &&
        weedout_end > static_cast<plan_idx>(idx)) {
      force_sort_position = true;
      break;
    }
  }

  explain_flags.set(sort_order->src, ESP_USING_FILESORT);
  QEP_TAB *const tab = &qep_tab[idx];
  bool keep_buffers =
      qep_tab->join() != nullptr &&
      qep_tab->join()->query_block->master_query_expression()->item !=
          nullptr &&
      qep_tab->join()
          ->query_block->master_query_expression()
          ->item->is_uncacheable();

  {
    // Switch to the right slice if applicable, so that we fetch out the correct
    // items from order_arg.
    Switch_ref_item_slice slice_switch(this, tab->ref_item_slice);
    tab->filesort = new (thd->mem_root)
        Filesort(thd, {tab->table()}, keep_buffers, sort_order->order,
                 HA_POS_ERROR, force_stable_sort,
                 /*remove_duplicates=*/false, force_sort_position,
                 /*unwrap_rollup=*/sort_before_group);
    tab->filesort_pushed_order = sort_order->order;
  }
  if (!tab->filesort) return true;
  Opt_trace_object trace_tmp(&thd->opt_trace, "filesort");
  trace_tmp.add_alnum("adding_sort_to_table",
                      tab->table() ? tab->table()->alias : "");

  return false;
}

/**
  Find a cheaper access key than a given key.

  @param          tab                 NULL or JOIN_TAB of the accessed table
  @param          order               Linked list of ORDER BY arguments
  @param          table               Table if tab == NULL or tab->table()
  @param          usable_keys         Key map to find a cheaper key in
  @param          ref_key
                * 0 <= key < MAX_KEY   - key number (hint) to start the search
                * -1                   - no key number provided
  @param          select_limit        LIMIT value, or HA_POS_ERROR if no limit
  @param [out]    new_key             Key number if success, otherwise undefined
  @param [out]    new_key_direction   Return -1 (reverse) or +1 if success,
                                      otherwise undefined
  @param [out]    new_select_limit    Return adjusted LIMIT
  @param [out]    new_used_key_parts  NULL by default, otherwise return number
                                      of new_key prefix columns if success
                                      or undefined if the function fails
  @param [out]  saved_best_key_parts  NULL by default, otherwise preserve the
                                      value for further use in QUICK_SELECT_DESC

  @note
    This function takes into account table->quick_condition_rows statistic
    (that is calculated by JOIN::make_join_plan()).
    However, single table procedures such as mysql_update() and mysql_delete()
    never call JOIN::make_join_plan(), so they have to update it manually
    (@see get_index_for_order()).
    This function resets bits in TABLE::quick_keys for indexes with mixed
    ASC/DESC keyparts as range scan doesn't support range reordering
    required for them.
*/

bool test_if_cheaper_ordering(const JOIN_TAB *tab, ORDER_with_src *order,
                              TABLE *table, Key_map usable_keys, int ref_key,
                              ha_rows select_limit, int *new_key,
                              int *new_key_direction, ha_rows *new_select_limit,
                              uint *new_used_key_parts,
                              uint *saved_best_key_parts) {
  DBUG_TRACE;
  /*
    Check whether there is an index compatible with the given order
    usage of which is cheaper than usage of the ref_key index (ref_key>=0)
    or a table scan.
    It may be the case if ORDER/GROUP BY is used with LIMIT.
  */
  ha_rows best_select_limit = HA_POS_ERROR;
  JOIN *join = tab ? tab->join() : nullptr;
  if (join) ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  uint nr;
  uint best_key_parts = 0;
  int best_key_direction = 0;
  ha_rows best_records = 0;
  double read_time;
  int best_key = -1;
  bool is_best_covering = false;
  double fanout = 1;
  ha_rows table_records = table->file->stats.records;
  bool group = join && join->grouped && order == &join->group_list;
  double refkey_rows_estimate =
      static_cast<double>(table->quick_condition_rows);
  const bool has_limit = (select_limit != HA_POS_ERROR);
  const join_type cur_access_method = tab ? tab->type() : JT_ALL;

  if (join) {
    read_time = tab->position()->read_cost;
    for (uint jt = tab->idx() + 1; jt < join->primary_tables; jt++) {
      POSITION *pos = join->best_ref[jt]->position();
      fanout *= pos->rows_fetched * pos->filter_effect;
      if (fanout < 0) break;  // fanout became 'unknown'
    }
  } else
    read_time = table->file->table_scan_cost().total_cost();

  /*
    Calculate the selectivity of the ref_key for REF_ACCESS. For
    RANGE_ACCESS we use table->quick_condition_rows.
  */
  if (ref_key >= 0 && cur_access_method == JT_REF) {
    if (table->quick_keys.is_set(ref_key))
      refkey_rows_estimate = static_cast<double>(table->quick_rows[ref_key]);
    else {
      const KEY *ref_keyinfo = table->key_info + ref_key;
      if (ref_keyinfo->has_records_per_key(tab->ref().key_parts - 1))
        refkey_rows_estimate =
            ref_keyinfo->records_per_key(tab->ref().key_parts - 1);
      else
        refkey_rows_estimate = 1.0;  // No index statistics
    }
    assert(refkey_rows_estimate >= 1.0);
  }

  for (nr = 0; nr < table->s->keys; nr++) {
    int direction = 0;
    uint used_key_parts;
    bool skip_quick;

    if (usable_keys.is_set(nr) &&
        (direction = test_if_order_by_key(order, table, nr, &used_key_parts,
                                          &skip_quick))) {
      bool is_covering = table->covering_keys.is_set(nr) ||
                         (nr == table->s->primary_key &&
                          table->file->primary_key_is_clustered());
      // Don't allow backward scans on indexes with mixed ASC/DESC key parts
      if (skip_quick) table->quick_keys.clear_bit(nr);

      /*
        Don't use an index scan with ORDER BY without limit.
        For GROUP BY without limit always use index scan
        if there is a suitable index.
        Why we hold to this asymmetry hardly can be explained
        rationally. It's easy to demonstrate that using
        temporary table + filesort could be cheaper for grouping
        queries too.
      */
      if (is_covering || select_limit != HA_POS_ERROR ||
          (ref_key < 0 && (group || table->force_index_order))) {
        rec_per_key_t rec_per_key;
        KEY *keyinfo = table->key_info + nr;
        if (select_limit == HA_POS_ERROR) select_limit = table_records;
        if (group) {
          /*
            Used_key_parts can be larger than keyinfo->key_parts
            when using a secondary index clustered with a primary
            key (e.g. as in Innodb).
            See Bug #28591 for details.
          */
          rec_per_key =
              used_key_parts && used_key_parts <= actual_key_parts(keyinfo)
                  ? keyinfo->records_per_key(used_key_parts - 1)
                  : 1.0f;
          rec_per_key = std::max(rec_per_key, 1.0f);
          /*
            With a grouping query each group containing on average
            rec_per_key records produces only one row that will
            be included into the result set.
          */
          if (select_limit > table_records / rec_per_key)
            select_limit = table_records;
          else
            select_limit = (ha_rows)(select_limit * rec_per_key);
        }
        /*
          If tab=tk is not the last joined table tn then to get first
          L records from the result set we can expect to retrieve
          only L/fanout(tk,tn) where fanout(tk,tn) says how many
          rows in the record set on average will match each row tk.
          Usually our estimates for fanouts are too pessimistic.
          So the estimate for L/fanout(tk,tn) will be too optimistic
          and as result we'll choose an index scan when using ref/range
          access + filesort will be cheaper.
        */
        if (fanout == 0)                // Would have been a division-by-zero
          select_limit = HA_POS_ERROR;  // -> 'infinite'
        else if (fanout > 0)            // 'fanout' not unknown
          select_limit =
              (ha_rows)(select_limit < fanout ? 1 : select_limit / fanout);
        /*
          We assume that each of the tested indexes is not correlated
          with ref_key. Thus, to select first N records we have to scan
          N/selectivity(ref_key) index entries.
          selectivity(ref_key) = #scanned_records/#table_records =
          refkey_rows_estimate/table_records.
          In any case we can't select more than #table_records.
          N/(refkey_rows_estimate/table_records) > table_records
          <=> N > refkey_rows_estimate.
          Neither should it be possible to select more than #table_records
          rows from refkey_rows_estimate.
         */
        if (select_limit > refkey_rows_estimate)
          select_limit = table_records;
        else if (table_records >= refkey_rows_estimate)
          select_limit = (ha_rows)(select_limit * (double)table_records /
                                   refkey_rows_estimate);
        rec_per_key =
            keyinfo->records_per_key(keyinfo->user_defined_key_parts - 1);
        rec_per_key = std::max(rec_per_key, 1.0f);
        /*
          Here we take into account the fact that rows are
          accessed in sequences rec_per_key records in each.
          Rows in such a sequence are supposed to be ordered
          by rowid/primary key. When reading the data
          in a sequence we'll touch not more pages than the
          table file contains.
          TODO. Use the formula for a disk sweep sequential access
          to calculate the cost of accessing data rows for one
          index entry.
        */
        const Cost_estimate table_scan_time = table->file->table_scan_cost();
        const double index_scan_time =
            select_limit / rec_per_key *
            min<double>(table->cost_model()->page_read_cost(rec_per_key),
                        table_scan_time.total_cost());

        /*
          Switch to index that gives order if its scan time is smaller than
          read_time of current chosen access method. In addition, if the
          current chosen access method is index scan or table scan, always
          switch to the index that gives order when it is covering or when
          force index order or group by is present.
        */
        if (((cur_access_method == JT_ALL ||
              cur_access_method == JT_INDEX_SCAN) &&
             (is_covering || group || table->force_index_order)) ||
            index_scan_time < read_time) {
          ha_rows quick_records = table_records;
          const ha_rows refkey_select_limit =
              (ref_key >= 0 && table->covering_keys.is_set(ref_key))
                  ? static_cast<ha_rows>(refkey_rows_estimate)
                  : HA_POS_ERROR;

          if ((is_best_covering && !is_covering) ||
              (is_covering && refkey_select_limit < select_limit))
            continue;
          if (table->quick_keys.is_set(nr))
            quick_records = table->quick_rows[nr];
          if (best_key < 0 ||
              (select_limit <= min(quick_records, best_records)
                   ? keyinfo->user_defined_key_parts < best_key_parts
                   : quick_records < best_records) ||
              // We assume forward scan is faster than backward even if the
              // key is longer. This should be taken into account in cost
              // calculation.
              direction > best_key_direction) {
            best_key = nr;
            best_key_parts = keyinfo->user_defined_key_parts;
            if (saved_best_key_parts) *saved_best_key_parts = used_key_parts;
            best_records = quick_records;
            is_best_covering = is_covering;
            best_key_direction = direction;
            best_select_limit = select_limit;
          }
        }
      }
    }
  }

  if (best_key < 0 || best_key == ref_key) return false;

  *new_key = best_key;
  *new_key_direction = best_key_direction;
  *new_select_limit = has_limit ? best_select_limit : table_records;
  if (new_used_key_parts != nullptr) *new_used_key_parts = best_key_parts;

  return true;
}

/**
  Find a key to apply single table UPDATE/DELETE by a given ORDER

  @param       order           Linked list of ORDER BY arguments
  @param       tab             Table to find a key
  @param       limit           LIMIT clause parameter
  @param [out] need_sort       true if filesort needed
  @param [out] reverse
    true if the key is reversed again given ORDER (undefined if key == MAX_KEY)

  @return
    - MAX_KEY if no key found                        (need_sort == true)
    - MAX_KEY if quick select result order is OK     (need_sort == false)
    - key number (either index scan or quick select) (need_sort == false)

  @note
    Side effects:
    - may deallocate or deallocate and replace select->quick;
    - may set table->quick_condition_rows and table->quick_rows[...]
      to table->file->stats.records.
*/

uint get_index_for_order(ORDER_with_src *order, QEP_TAB *tab, ha_rows limit,
                         bool *need_sort, bool *reverse) {
  if (tab->quick() &&
      tab->quick()->unique_key_range()) {  // Single row select (always
                                           // "ordered"): Ok to use with key
                                           // field UPDATE
    *need_sort = false;
    /*
      Returning of MAX_KEY here prevents updating of used_key_is_modified
      in mysql_update(). Use quick select "as is".
    */
    return MAX_KEY;
  }

  TABLE *const table = tab->table();

  if (order->empty()) {
    *need_sort = false;
    if (tab->quick())
      return tab->quick()->index;  // index or MAX_KEY, use quick select as is
    else
      return table->file
          ->key_used_on_scan;  // MAX_KEY or index for some engines
  }

  if (!is_simple_order(order->order))  // just to cut further expensive checks
  {
    *need_sort = true;
    return MAX_KEY;
  }

  if (tab->quick()) {
    if (tab->quick()->index == MAX_KEY) {
      *need_sort = true;
      return MAX_KEY;
    }

    uint used_key_parts;
    bool skip_quick;
    switch (test_if_order_by_key(order, table, tab->quick()->index,
                                 &used_key_parts, &skip_quick)) {
      case 1:  // desired order
        *need_sort = false;
        return tab->quick()->index;
      case 0:  // unacceptable order
        *need_sort = true;
        return MAX_KEY;
      case -1:  // desired order, but opposite direction
      {
        QUICK_SELECT_I *reverse_quick;
        if (!skip_quick &&
            (reverse_quick = tab->quick()->make_reverse(used_key_parts))) {
          delete tab->quick();
          tab->set_quick(reverse_quick);
          tab->set_type(calc_join_type(reverse_quick->get_type()));
          *need_sort = false;
          return reverse_quick->index;
        } else {
          *need_sort = true;
          return MAX_KEY;
        }
      }
    }
    assert(0);
  } else if (limit != HA_POS_ERROR) {  // check if some index scan & LIMIT is
                                       // more efficient than filesort

    /*
      Update quick_condition_rows since single table UPDATE/DELETE procedures
      don't call JOIN::make_join_plan() and leave this variable uninitialized.
    */
    table->quick_condition_rows = table->file->stats.records;

    int key, direction;
    if (test_if_cheaper_ordering(nullptr, order, table,
                                 table->keys_in_use_for_order_by, -1, limit,
                                 &key, &direction, &limit)) {
      *need_sort = false;
      *reverse = (direction < 0);
      return key;
    }
  }
  *need_sort = true;
  return MAX_KEY;
}

/**
  Returns number of key parts depending on
  OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS flag.

  @param  key_info  pointer to KEY structure

  @return number of key parts.
*/

uint actual_key_parts(const KEY *key_info) {
  return current_thd->optimizer_switch_flag(
             OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS)
             ? key_info->actual_key_parts
             : key_info->user_defined_key_parts;
}

uint actual_key_flags(const KEY *key_info) {
  return current_thd->optimizer_switch_flag(
             OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS)
             ? key_info->actual_flags
             : key_info->flags;
}

join_type calc_join_type(int quick_type) {
  if ((quick_type == QUICK_SELECT_I::QS_TYPE_INDEX_MERGE) ||
      (quick_type == QUICK_SELECT_I::QS_TYPE_ROR_INTERSECT) ||
      (quick_type == QUICK_SELECT_I::QS_TYPE_ROR_UNION))
    return JT_INDEX_MERGE;
  else
    return JT_RANGE;
}

/**
  @} (end of group Query_Optimizer)


// Source: sql_select.cc
// Lines 2935-5301
