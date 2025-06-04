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


// Source: sql_select.cc
// Lines 3121-3318
