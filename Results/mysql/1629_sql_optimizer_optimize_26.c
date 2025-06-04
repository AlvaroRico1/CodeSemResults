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


// Source: sql_optimizer.cc
// Lines 255-332
