bool Query_block::push_conditions_to_derived_tables(THD *thd) {
  if (materialized_derived_table_count > 0)
    for (TABLE_LIST *tl = leaf_tables; tl; tl = tl->next_leaf) {
      if (tl->is_view_or_derived() && tl->uses_materialization() &&
          where_cond() && tl->can_push_condition_to_derived(thd)) {
        Item **where = where_cond_ref();
        Opt_trace_context *const trace = &thd->opt_trace;
        Condition_pushdown cp(*where, tl, thd, trace);
        // Make condition for the derived table
        if (cp.make_cond_for_derived()) return true;
        // The remaining condition that could not be pushed stays in this
        // WHERE clause.
        *where = cp.get_remainder_cond();
      }
    }
  /*
    Push conditions if possible to derived tables which were not merged. By
    running top-down, the resulting pushed down condition can be pushed down
    even more, in the case where a derived table contains an inner derived
    table.
   */
  for (Query_expression *unit = first_inner_query_expression(); unit;
       unit = unit->next_query_expression()) {
    for (Query_block *sl = unit->first_query_block(); sl;
         sl = sl->next_query_block()) {
      if (sl->push_conditions_to_derived_tables(thd)) return true;
    }
  }
  return false;
}


// Source: sql_resolver.cc
// Lines 599-628
