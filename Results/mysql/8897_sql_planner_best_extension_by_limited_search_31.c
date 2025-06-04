bool Optimize_table_order::best_extension_by_limited_search(
    table_map remaining_tables, uint idx, uint current_search_depth) {
  DBUG_TRACE;

  DBUG_EXECUTE_IF("bug13820776_2", thd->killed = THD::KILL_QUERY;);
  if (thd->killed)  // Abort
    return true;

  const Cost_model_server *const cost_model = join->cost_model();
  Opt_trace_context *const trace = &thd->opt_trace;

  /*
     'join' is a partial plan with lower cost than the best plan so far,
     so continue expanding it further with the tables in 'remaining_tables'.
  */
  double best_rowcount = DBL_MAX;
  double best_cost = DBL_MAX;

  DBUG_EXECUTE("opt",
               print_plan(join, idx,
                          idx ? join->positions[idx - 1].prefix_rowcount : 1.0,
                          idx ? join->positions[idx - 1].prefix_cost : 0.0,
                          idx ? join->positions[idx - 1].prefix_cost : 0.0,
                          "part_plan"););

  /*
    'eq_ref_extended' are the 'remaining_tables' which has already been
    involved in an partial query plan extension if this QEP. These
    will not be considered in further EQ_REF extensions based
    on current (partial) QEP.
  */
  table_map eq_ref_extended(0);

  JOIN_TAB *saved_refs[MAX_TABLES];
  // Save 'best_ref[]' as we has to restore before return.
  memcpy(saved_refs, join->best_ref + idx,
         sizeof(JOIN_TAB *) * (join->tables - idx));

  Deps_of_remaining_lateral_derived_tables deps_lateral(join, ~excluded_tables);

  for (JOIN_TAB **pos = join->best_ref + idx; *pos && !use_best_so_far; pos++) {
    JOIN_TAB *const s = *pos;
    const table_map real_table_bit = s->table_ref->map();

    /*
      Don't move swap inside conditional code: All items should
      be uncond. swapped to maintain '#rows-ordered' best_ref[].
      This is critical for early pruning of bad plans.
    */
    std::swap(join->best_ref[idx], *pos);

    if ((remaining_tables & real_table_bit) &&
        !(eq_ref_extended & real_table_bit) &&
        !(remaining_tables & s->dependent) &&
        (!idx || !check_interleaving_with_nj(s))) {
      Opt_trace_object trace_one_table(trace);
      if (unlikely(trace->is_started())) {
        trace_plan_prefix(join, idx, excluded_tables);
        trace_one_table.add_utf8_table(s->table_ref);
      }
      POSITION *const position = join->positions + idx;

      // If optimizing a sj-mat nest, tables in this plan must be in nest:
      assert(emb_sjm_nest == nullptr || emb_sjm_nest == s->emb_sj_nest);

      deps_lateral.restore();  // as we "popped" the previously-tried table

      /* Find the best access method from 's' to the current partial plan */
      best_access_path(s, remaining_tables, idx, false,
                       idx ? (position - 1)->prefix_rowcount : 1.0, position);

      // Compute the cost of extending the plan with 's'
      position->set_prefix_join_cost(idx, cost_model);

      trace_one_table
          .add("condition_filtering_pct", position->filter_effect * 100)
          .add("rows_for_plan", position->prefix_rowcount)
          .add("cost_for_plan", position->prefix_cost);

      if (has_sj) {
        /*
          Even if there are no semijoins, advance_sj_state() has a significant
          cost (takes 9% of time in a 20-table plan search), hence the if()
          above, which is also more efficient than the same if() inside
          advance_sj_state() would be.
          Besides, never call advance_sj_state() when calculating the plan
          for a materialized semi-join nest.
        */
        advance_sj_state(remaining_tables, s, idx);
      } else
        position->no_semijoin();

      /*
        Expand only partial plans with lower cost than the best QEP so far.
        However, if the best plan so far uses a disabled semi-join strategy,
        we continue the search since this partial plan may support other
        semi-join strategies.
      */
      if (position->prefix_cost >= join->best_read &&
          found_plan_with_allowed_sj) {
        DBUG_EXECUTE("opt",
                     print_plan(join, idx + 1, position->prefix_rowcount,
                                position->read_cost, position->prefix_cost,
                                "prune_by_cost"););
        trace_one_table.add("pruned_by_cost", true);
        backout_nj_state(remaining_tables, s);
        continue;
      }

      /*
        Prune some less promising partial plans. This heuristic may miss
        the optimal QEPs, thus it results in a non-exhaustive search.
      */
      if (prune_level == 1) {
        if (best_rowcount > position->prefix_rowcount ||
            best_cost > position->prefix_cost ||
            (idx == join->const_tables &&  // 's' is the first table in the QEP
             s->table() == join->sort_by_table)) {
          if (best_rowcount >= position->prefix_rowcount &&
              best_cost >= position->prefix_cost &&
              /* TODO: What is the reasoning behind this condition? */
              (!(s->key_dependent & remaining_tables) ||
               position->rows_fetched < 2.0)) {
            best_rowcount = position->prefix_rowcount;
            best_cost = position->prefix_cost;
          }
        } else if (found_plan_with_allowed_sj) {
          DBUG_EXECUTE("opt",
                       print_plan(join, idx + 1, position->prefix_rowcount,
                                  position->read_cost, position->prefix_cost,
                                  "pruned_by_heuristic"););
          trace_one_table.add("pruned_by_heuristic", true);
          backout_nj_state(remaining_tables, s);
          continue;
        }
      }

      deps_lateral.recalculate(s, idx + 1);

      const table_map remaining_tables_after =
          (remaining_tables & ~real_table_bit);
      if ((current_search_depth > 1) && remaining_tables_after) {
        /*
          Explore more extensions of plan:
          If possible, use heuristic to avoid a full expansion of partial QEP.
          Evaluate a simplified EQ_REF extension of QEP if:
            1) Pruning is enabled.
            2) and, There are tables joined by (EQ_)REF key.
            3) and, There is a 1::1 relation between those tables
        */
        if (prune_level == 1 &&             // 1)
            position->key != nullptr &&     // 2)
            position->rows_fetched <= 1.0)  // 3)
        {
          /*
            Join in this 'position' is an EQ_REF-joined table, append more
            EQ_REFs. We do this only for the first EQ_REF we encounter which
            will then include other EQ_REFs from 'remaining_tables' and inform
            about which tables was 'eq_ref_extended'. These are later 'pruned'
            as they was processed here.
          */
          if (eq_ref_extended == (table_map)0) {
            /* Try an EQ_REF-joined expansion of the partial plan */
            Opt_trace_array trace_rest(trace, "rest_of_plan");
            eq_ref_extended =
                real_table_bit |
                eq_ref_extension_by_limited_search(
                    remaining_tables_after, idx + 1, current_search_depth - 1);
            if (eq_ref_extended == ~(table_map)0) return true;  // Failed

            backout_nj_state(remaining_tables, s);

            if (eq_ref_extended == remaining_tables) goto done;

            continue;
          } else  // Skip, as described above
          {
            DBUG_EXECUTE("opt",
                         print_plan(join, idx + 1, position->prefix_rowcount,
                                    position->read_cost, position->prefix_cost,
                                    "pruned_by_eq_ref_heuristic"););
            trace_one_table.add("pruned_by_eq_ref_heuristic", true);
            backout_nj_state(remaining_tables, s);
            continue;
          }
        }  // if (prunable...)

        /* Fallthrough: Explore more best extensions of plan */
        Opt_trace_array trace_rest(trace, "rest_of_plan");
        if (best_extension_by_limited_search(remaining_tables_after, idx + 1,
                                             current_search_depth - 1))
          return true;
      } else  // if ((current_search_depth > 1) && ...
      {
        if (consider_plan(idx, &trace_one_table)) return true;
        /*
          If plan is complete, there should be no "open" outer join nest, and
          all semi join nests should be handled by a strategy:
        */
        assert((remaining_tables_after != 0) ||
               ((cur_embedding_map == 0) &&
                (join->positions[idx].dups_producing_tables == 0) &&
                (join->deps_of_remaining_lateral_derived_tables == 0)));
      }
      backout_nj_state(remaining_tables, s);
    }
  }


// Source: sql_planner.cc
// Lines 2686-2892
