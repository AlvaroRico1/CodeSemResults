table_map Optimize_table_order::eq_ref_extension_by_limited_search(
    table_map remaining_tables, uint idx, uint current_search_depth) {
  DBUG_TRACE;

  if (remaining_tables == 0) return 0;

  /*
    The section below adds 'eq_ref' joinable tables to the QEP in the order
    they are found in the 'remaining_tables' set.
    See above description for why we can add these without greedy
    cost analysis.
  */
  Opt_trace_context *const trace = &thd->opt_trace;
  table_map eq_ref_ext(0);
  JOIN_TAB *s;
  JOIN_TAB *saved_refs[MAX_TABLES];
  // Save 'best_ref[]' as we has to restore before return.
  memcpy(saved_refs, join->best_ref + idx,
         sizeof(JOIN_TAB *) * (join->tables - idx));

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);

  for (JOIN_TAB **pos = join->best_ref + idx; (s = *pos); pos++) {
    const table_map real_table_bit = s->table_ref->map();

    /*
      Don't move swap inside conditional code: All items
      should be swapped to maintain '#rows' ordered tables.
      This is critical for early pruning of bad plans.
    */
    std::swap(join->best_ref[idx], *pos);

    /*
      Consider table for 'eq_ref' heuristic if:
        1)      It might use a keyref for best_access_path
        2) and, Table remains to be handled.
        3) and, It is independent of those not yet in partial plan.
        4) and, It is key dependent on at least one already handled table
        5) and, It passed the interleaving check.
    */
    if (s->keyuse() &&                             // 1)
        (remaining_tables & real_table_bit) &&     // 2)
        !(remaining_tables & s->dependent) &&      // 3)
        (~remaining_tables & s->key_dependent) &&  // 4)
        (!idx || !check_interleaving_with_nj(s)))  // 5)
    {
      Opt_trace_object trace_one_table(trace);
      if (unlikely(trace->is_started())) {
        trace_plan_prefix(join, idx, excluded_tables);
        trace_one_table.add_utf8_table(s->table_ref);
      }
      POSITION *const position = join->positions + idx;

      assert(emb_sjm_nest == nullptr || emb_sjm_nest == s->emb_sj_nest);

      deps_lateral.restore();

      /* Find the best access method from 's' to the current partial plan */
      best_access_path(s, remaining_tables, idx, false,
                       idx ? (position - 1)->prefix_rowcount : 1.0, position);

      /*
        EQ_REF prune logic is based on that all joins
        in the ref_extension has the same #rows and cost.
        -> The total cost of the QEP is independent of the order
           of joins within this 'ref_extension'.
           Expand QEP with all 'identical' REFs in
          'join->positions' order.
        Note that due to index statistics from the storage engines
        is a floating point number and might not be exact, the
        rows and cost estimates for eq_ref on two tables might not
        be the exact same number.
        @todo This test could likely be re-implemented to use
        information about whether the index is unique or not.
      */
      const bool added_to_eq_ref_extension =
          position->key &&
          almost_equal(position->read_cost, (position - 1)->read_cost) &&
          almost_equal(position->rows_fetched, (position - 1)->rows_fetched);
      trace_one_table.add("added_to_eq_ref_extension",
                          added_to_eq_ref_extension);
      if (added_to_eq_ref_extension) {
        // Add the cost of extending the plan with 's'
        position->set_prefix_join_cost(idx, join->cost_model());

        trace_one_table
            .add("condition_filtering_pct", position->filter_effect * 100)
            .add("rows_for_plan", position->prefix_rowcount)
            .add("cost_for_plan", position->prefix_cost);

        if (has_sj) {
          /*
            Even if there are no semijoins, advance_sj_state() has a
            significant cost (takes 9% of time in a 20-table plan search),
            hence the if() above, which is also more efficient than the
            same if() inside advance_sj_state() would be.
          */
          advance_sj_state(remaining_tables, s, idx);
        } else
          position->no_semijoin();

        // Expand only partial plans with lower cost than the best QEP so far
        if (position->prefix_cost >= join->best_read) {
          DBUG_EXECUTE("opt",
                       print_plan(join, idx + 1, position->prefix_rowcount,
                                  position->read_cost, position->prefix_cost,
                                  "prune_by_cost"););
          trace_one_table.add("pruned_by_cost", true);
          backout_nj_state(remaining_tables, s);
          continue;
        }

        deps_lateral.recalculate(s, idx + 1);

        eq_ref_ext = real_table_bit;
        const table_map remaining_tables_after =
            (remaining_tables & ~real_table_bit);
        if ((current_search_depth > 1) && remaining_tables_after) {
          DBUG_EXECUTE("opt",
                       print_plan(join, idx + 1, position->prefix_rowcount,
                                  position->read_cost, position->prefix_cost,
                                  "EQ_REF_extension"););

          /* Recursively EQ_REF-extend the current partial plan */
          Opt_trace_array trace_rest(trace, "rest_of_plan");
          eq_ref_ext |= eq_ref_extension_by_limited_search(
              remaining_tables_after, idx + 1, current_search_depth - 1);
        } else {
          if (consider_plan(idx, &trace_one_table)) return ~(table_map)0;
          assert((remaining_tables_after != 0) ||
                 ((cur_embedding_map == 0) &&
                  (join->positions[idx].dups_producing_tables == 0) &&
                  (join->deps_of_remaining_lateral_derived_tables == 0)));
        }
        backout_nj_state(remaining_tables, s);
        memcpy(join->best_ref + idx, saved_refs,
               sizeof(JOIN_TAB *) * (join->tables - idx));
        return eq_ref_ext;
      }  // if (added_to_eq_ref_extension)

      backout_nj_state(remaining_tables, s);
    }  // if (... !check_interleaving_with_nj() ...)
  }    // for (JOIN_TAB **pos= ...)

  memcpy(join->best_ref + idx, saved_refs,
         sizeof(JOIN_TAB *) * (join->tables - idx));
  deps_lateral.restore();
  /*
    'eq_ref' heuristic didn't find a table to be appended to
    the query plan. We need to use the greedy search
    for finding the next table to be added.
  */
  assert(!eq_ref_ext);
  if (best_extension_by_limited_search(remaining_tables, idx,
                                       current_search_depth))
    return ~(table_map)0;

  return eq_ref_ext;
}


// Source: sql_planner.cc
// Lines 3034-3192
