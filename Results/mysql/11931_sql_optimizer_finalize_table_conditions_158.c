bool JOIN::finalize_table_conditions() {
  /*
    Unnecessary to reduce conditions for const tables as they are only
    evaluated once.
  */
  assert(!plan_is_const());
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_tables(trace, "finalizing_table_conditions");

  for (uint i = const_tables; i < tables; i++) {
    Item *condition = best_ref[i]->condition();
    if (condition == nullptr) continue;

    /*
      Table predicates known to be true by the selected
      (ref-)access method may be removed from the condition
    */
    Opt_trace_object trace_cond(trace);
    trace_cond.add_utf8_table(best_ref[i]->table_ref);
    trace_cond.add("original_table_condition", condition);

    /*
      Calculate the set of possibly NULL extended tables when 'condition'
      is evaluated. As it is evaluated on a found row from table, that
      table is subtracted from the nullable tables. Note that a FOUND_MATCH
      trigger is a special case, handled in reduce_cond_for_table().
    */
    const table_map null_extended =
        query_block->outer_join & ~best_ref[i]->table_ref->map();
    condition = reduce_cond_for_table(condition, null_extended);
    if (condition != nullptr) condition->update_used_tables();

    /*
      Cache constant expressions in table conditions.
      (Moved down from WHERE- and ON-clauses)
    */
    if (condition != nullptr) {
      cache_const_expr_arg cache_arg;
      cache_const_expr_arg *analyzer_arg = &cache_arg;
      condition = condition->compile(
          &Item::cache_const_expr_analyzer, (uchar **)&analyzer_arg,
          &Item::cache_const_expr_transformer, (uchar *)&cache_arg);
      if (condition == nullptr) return true;
    }


// Source: sql_optimizer.cc
// Lines 8748-8794
