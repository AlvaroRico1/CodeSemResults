void Optimize_table_order::optimize_straight_join(table_map join_tables) {
  uint idx = join->const_tables;
  double rowcount = 1.0;
  double cost = 0.0;
  const Cost_model_server *const cost_model = join->cost_model();

  // resolve_subquery() disables semijoin if STRAIGHT_JOIN
  assert(join->query_block->sj_nests.empty());

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);

  Opt_trace_context *const trace = &join->thd->opt_trace;
  for (JOIN_TAB **pos = join->best_ref + idx; *pos; idx++, pos++) {
    JOIN_TAB *const s = *pos;
    POSITION *const position = join->positions + idx;
    Opt_trace_object trace_table(trace);
    if (unlikely(trace->is_started())) {
      trace_plan_prefix(join, idx, excluded_tables);
      trace_table.add_utf8_table(s->table_ref);
    }
    /*
      Dependency computation (JOIN::make_join_plan()) and proper ordering
      based on them (join_tab_cmp*) guarantee that this order is compatible
      with execution, check it:
    */
    assert(!check_interleaving_with_nj(s));

    /* Find the best access method from 's' to the current partial plan */
    best_access_path(s, join_tables, idx, false, rowcount, position);

    // compute the cost of the new plan extended with 's'
    position->set_prefix_join_cost(idx, cost_model);

    position->no_semijoin();  // advance_sj_state() is not needed

    rowcount = position->prefix_rowcount;
    cost = position->prefix_cost;

    trace_table.add("condition_filtering_pct", position->filter_effect * 100)
        .add("rows_for_plan", rowcount)
        .add("cost_for_plan", cost);
    join_tables &= ~(s->table_ref->map());

    deps_lateral.recalculate(s, idx + 1);
  }


// Source: sql_planner.cc
// Lines 2077-2121
