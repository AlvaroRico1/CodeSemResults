void Optimize_table_order::advance_sj_state(table_map remaining_tables,
                                            const JOIN_TAB *new_join_tab,
                                            uint idx) {
  Opt_trace_context *const trace = &thd->opt_trace;
  TABLE_LIST *const emb_sj_nest = new_join_tab->emb_sj_nest;
  POSITION *const pos = join->positions + idx;
  double best_cost = pos->prefix_cost;
  double best_rowcount = pos->prefix_rowcount;
  uint sj_strategy = SJ_OPT_NONE;  // Initially: No chosen strategy

  /*
    Semi-join nests cannot be nested, hence we never need to advance the
    semi-join state of a materialized semi-join query.
    In fact, doing this may cause undesirable effects because all tables
    within a semi-join nest have emb_sj_nest != NULL, which triggers several
    of the actions inside this function.
  */
  assert(emb_sjm_nest == nullptr);

  // remaining_tables include the current one:
  assert(remaining_tables & new_join_tab->table_ref->map());
  // Save it:
  const table_map remaining_tables_incl = remaining_tables;
  // And add the current table to the join prefix:
  remaining_tables &= ~new_join_tab->table_ref->map();

  DBUG_TRACE;

  Opt_trace_array trace_choices(trace, "semijoin_strategy_choice");

  /* Initialize the state or copy it from prev. tables */
  pos->cur_embedding_map = cur_embedding_map;
  if (idx == join->const_tables) {
    pos->dups_producing_tables = 0;
    pos->first_firstmatch_table = MAX_TABLES;
    pos->first_loosescan_table = MAX_TABLES;
    pos->dupsweedout_tables = 0;
    pos->sjm_scan_need_tables = 0;
    pos->sjm_scan_last_inner = 0;
  } else {
    pos->dups_producing_tables = pos[-1].dups_producing_tables;

    // FirstMatch
    pos->first_firstmatch_table = pos[-1].first_firstmatch_table;
    pos->first_firstmatch_rtbl = pos[-1].first_firstmatch_rtbl;
    pos->firstmatch_need_tables = pos[-1].firstmatch_need_tables;

    // LooseScan
    pos->first_loosescan_table = (pos[-1].sj_strategy == SJ_OPT_LOOSE_SCAN)
                                     ? MAX_TABLES
                                     : pos[-1].first_loosescan_table;
    pos->loosescan_need_tables = pos[-1].loosescan_need_tables;

    // MaterializeScan
    pos->sjm_scan_need_tables = (pos[-1].sj_strategy == SJ_OPT_MATERIALIZE_SCAN)
                                    ? 0
                                    : pos[-1].sjm_scan_need_tables;
    pos->sjm_scan_last_inner = pos[-1].sjm_scan_last_inner;

    // Duplicate Weedout
    pos->dupsweedout_tables = pos[-1].dupsweedout_tables;
    pos->first_dupsweedout_table = pos[-1].first_dupsweedout_table;
  }

  table_map handled_by_fm_or_ls = 0;
  /*
    FirstMatch Strategy
    ===================

    FirstMatch requires that all dependent outer tables are in the join prefix.
    (see "FirstMatch strategy" above setup_semijoin_dups_elimination()).
    The execution strategy will handle multiple semi-join nests correctly,
    and the optimizer will pick execution strategy according to these rules:
    - If tables from multiple semi-join nests are intertwined, they will
      be processed as one FirstMatch evaluation.
    - If tables from each semi-join nest are grouped together, each semi-join
      nest is processed as one FirstMatch evaluation.

    Example: Let's say we have an outer table ot and two semi-join nests with
    two tables each: it11 and it12, and it21 and it22.

    Intertwined tables: ot - FM(it11 - it21 - it12 - it22)
    Grouped tables: ot - FM(it11 - it12) - FM(it21 - it22)
  */

  if (pos->first_firstmatch_table != MAX_TABLES) {
    const TABLE_LIST *first_emb_sj_nest =
        join->positions[pos->first_firstmatch_table].table->emb_sj_nest;
    if (emb_sj_nest != first_emb_sj_nest) {
      // Can't handle interleaving between tables from the
      // semi-join that FirstMatch is handling and any other tables.
      pos->first_firstmatch_table = MAX_TABLES;
    }
  }

  if (emb_sj_nest && emb_sj_nest->nested_join->sj_enabled_strategies &
                         OPTIMIZER_SWITCH_FIRSTMATCH) {
    const table_map outer_corr_tables = emb_sj_nest->nested_join->sj_depends_on;
    const table_map sj_inner_tables = emb_sj_nest->sj_inner_tables;
    /*
      Enter condition:
       1. The next join tab belongs to semi-join nest
          (verified for the encompassing code block above).
       2. We're not in a duplicate producer range yet
       3. All outer tables that
           - the subquery is correlated with, or
           - referred to from the outer_expr
          are in the join prefix
    */
    if (pos->dups_producing_tables == 0 &&        // (2)
        !(remaining_tables & outer_corr_tables))  // (3)
    {
      /* Start tracking potential FirstMatch range */
      pos->first_firstmatch_table = idx;
      pos->firstmatch_need_tables = 0;
      pos->first_firstmatch_rtbl = remaining_tables;
      // All inner tables should still be part of remaining_tables_inc
      assert(sj_inner_tables == (remaining_tables_incl & sj_inner_tables));
    }

    if (pos->first_firstmatch_table != MAX_TABLES) {
      /* Record that we need all of this semi-join's inner tables */
      pos->firstmatch_need_tables |= sj_inner_tables;

      if (outer_corr_tables & pos->first_firstmatch_rtbl) {
        /*
          Trying to add an sj-inner table whose sj-nest has an outer correlated
          table that was not in the prefix. This means FirstMatch can't be used.
        */
        pos->first_firstmatch_table = MAX_TABLES;
      } else if (!(pos->firstmatch_need_tables & remaining_tables)) {
        // Got a complete FirstMatch range.

        // We cannot FirstMatch to a different embedding nest,
        // e.g., for B LEFT JOIN (C SEMIJOIN D ON B.X=D.Y) and table order
        // B-D-C we cannot jump from D to B. This would cause non-hierarchical
        // joins. So we check that the jump won't leave from a still-open
        // nest: cur_embedding_map at the last table of this firstmatch range
        // must be included in cur_embedding_map at the target of the jump.
        nested_join_map cur_embedding_map_at_jump_target =
            pos->first_firstmatch_table > join->const_tables
                ? join->positions[pos->first_firstmatch_table - 1]
                      .cur_embedding_map
                : 0;
        if ((cur_embedding_map_at_jump_target & cur_embedding_map) !=
            cur_embedding_map) {
          pos->first_firstmatch_table = MAX_TABLES;
        } else {
          // Calculate access paths and cost
          double cost, rowcount;
          /* We use the same FirstLetterUpcase as in EXPLAIN */
          Opt_trace_object trace_one_strategy(trace);
          trace_one_strategy.add_alnum("strategy", "FirstMatch");
          (void)semijoin_firstmatch_loosescan_access_paths(
              pos->first_firstmatch_table, idx, remaining_tables, false,
              &rowcount, &cost);
          /*
            We don't yet know what are the other strategies, so pick FirstMatch.

            We ought to save the alternate POSITIONs produced by
            semijoin_firstmatch_loosescan_access_paths() but the problem is that
            providing save space uses too much space.
            Instead, we will re-calculate the alternate POSITIONs after we've
            picked the best QEP.
          */
          sj_strategy = SJ_OPT_FIRST_MATCH;
          best_cost = cost;
          best_rowcount = rowcount;
          trace_one_strategy.add("cost", best_cost).add("rows", best_rowcount);
          handled_by_fm_or_ls = pos->firstmatch_need_tables;

          trace_one_strategy.add("chosen", true);
        }
      }
    }
  }
  /*
    LooseScan Strategy
    ==================

    LooseScan requires that all dependent outer tables are not in the join
    prefix. (see "LooseScan strategy" above setup_semijoin_dups_elimination()).
    The tables must come in a rather strictly defined order:
    1. The LooseScan driving table (which is a subquery inner table).
    2. The remaining tables from the same semi-join nest as the above table.
    3. The outer dependent tables, possibly mixed with outer non-dependent
       tables.
    Notice that any other semi-joined tables must be outside this table range.
  */
  {
    if (pos->first_loosescan_table != MAX_TABLES) {
      const TABLE_LIST *first_emb_sj_nest =
          join->positions[pos->first_loosescan_table].table->emb_sj_nest;
      if (first_emb_sj_nest->sj_inner_tables & remaining_tables_incl) {
        // Stage 2: Accept remaining tables from the semi-join nest:
        if (emb_sj_nest != first_emb_sj_nest) {
          /*
            LooseScan strategy can't handle interleaving between tables from
            the semi-join that LooseScan is handling and any other tables.
          */
          pos->first_loosescan_table = MAX_TABLES;
        } else {
          /*
            NestedLoopSemiJoinWithDuplicateRemovalIterator takes a
            single-table iterator as left argument, and inner-joins
            it with the set of other SJ-inner tables. E.g. it doesn't work for
            A SEMI JOIN (B LEFT JOIN C) with B as LooseScan table. So:
            - if we're now at the second SJ-inner table (1) , and
            - this table belongs to a join nest which is outer-joined to
            the first SJ-inner table (2), or is directly outer-joined to the
            first SJ-inner table (3),
            - then both tables are not inner-joined together and LooseScan is
            impossible.
          */
          if (idx == pos->first_loosescan_table + 1 &&  // (1)
              ((pos->table->table_ref->outer_join_nest() !=
                join->positions[pos->first_loosescan_table]
                    .table->table_ref->outer_join_nest())  // (2)
               || pos->table->table_ref->outer_join))      // (3)
            pos->first_loosescan_table = MAX_TABLES;
        }
      } else {
        // Stage 3: Accept outer dependent and non-dependent tables:
        assert(emb_sj_nest != first_emb_sj_nest);
        if (emb_sj_nest != nullptr) pos->first_loosescan_table = MAX_TABLES;
      }
    }

    /*
      We may consider the LooseScan strategy if
      1a. The next table is an SJ-inner table, and
      1b. LooseScan is enabled for this SJ nest, and
      2. We have no more than 64 IN expressions (must fit in bitmap), and
      3. It is the first table from that semijoin, and
      4. We're not within a semi-join range, except
      new_join_tab->emb_sj_nest (which we've just entered, see #3), and
      5. All non-IN-equality correlation references from this sj-nest are
      bound, and
      6. But some of the IN-equalities aren't (so this can't be handled by
      FirstMatch strategy), and
      7. There are equalities (including maybe semi-join ones) which can be
      handled with an index of this table, and
      8. Not a derived table/view. (a temporary restriction)
    */
    if (emb_sj_nest &&  // (1a)
        emb_sj_nest->nested_join->sj_enabled_strategies &
            OPTIMIZER_SWITCH_LOOSE_SCAN &&                          // (1b)
        emb_sj_nest->nested_join->sj_inner_exprs.size() <= 64 &&    // (2)
        ((remaining_tables_incl & emb_sj_nest->sj_inner_tables) ==  // (3)
         emb_sj_nest->sj_inner_tables) &&                           // (3)
        pos->dups_producing_tables == 0 &&                          // (4)
        !(remaining_tables_incl &
          emb_sj_nest->nested_join->sj_corr_tables) &&  // (5)
        (remaining_tables_incl &
         emb_sj_nest->nested_join->sj_depends_on) &&       // (6)
        new_join_tab->keyuse() != nullptr &&               // (7)
        !new_join_tab->table_ref->uses_materialization())  // (8)
    {
      // start considering using LooseScan strategy
      pos->first_loosescan_table = idx;
      pos->loosescan_need_tables = emb_sj_nest->sj_inner_tables |
                                   emb_sj_nest->nested_join->sj_depends_on;
    }

    if ((pos->first_loosescan_table != MAX_TABLES) &&
        !(remaining_tables & pos->loosescan_need_tables)) {
      /*
        Ok we have all LooseScan sj-nest's inner tables and outer correlated
        tables into the prefix.
      */

      // Got a complete LooseScan range. Calculate access paths and cost
      double cost, rowcount;
      Opt_trace_object trace_one_strategy(trace);
      trace_one_strategy.add_alnum("strategy", "LooseScan");
      /*
        The same problem as with FirstMatch - we need to save POSITIONs
        somewhere but reserving space for all cases would require too
        much space. We will re-calculate POSITION structures later on.
        If this function returns 'false', it means LS is impossible (didn't
        find a suitable index, etc).
      */
      if (semijoin_firstmatch_loosescan_access_paths(pos->first_loosescan_table,
                                                     idx, remaining_tables,
                                                     true, &rowcount, &cost)) {
        /*
          We don't yet have any other strategies that could handle this
          semi-join nest (the other options are Duplicate Elimination or
          Materialization, which need at least the same set of tables in
          the join prefix to be considered) so unconditionally pick the
          LooseScan.
        */
        sj_strategy = SJ_OPT_LOOSE_SCAN;
        best_cost = cost;
        best_rowcount = rowcount;
        trace_one_strategy.add("cost", best_cost).add("rows", best_rowcount);
        handled_by_fm_or_ls = join->positions[pos->first_loosescan_table]
                                  .table->emb_sj_nest->sj_inner_tables;
      }
      trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_LOOSE_SCAN);
    }
  }

  if (emb_sj_nest) pos->dups_producing_tables |= emb_sj_nest->sj_inner_tables;

  pos->dups_producing_tables &= ~handled_by_fm_or_ls;

  /* MaterializeLookup and MaterializeScan strategy handler */
  const int sjm_strategy = semijoin_order_allows_materialization(
      join, remaining_tables, new_join_tab, idx);
  if (sjm_strategy == SJ_OPT_MATERIALIZE_SCAN) {
    /*
      We cannot evaluate this option now. This is because we cannot
      account for fanout of sj-inner tables yet:

        ntX  SJM-SCAN(it1 ... itN) | ot1 ... otN  |
                                   ^(1)           ^(2)

      we're now at position (1). SJM temptable in general has multiple
      records, so at point (1) we'll get the fanout from sj-inner tables (ie
      there will be multiple record combinations).

      The final join result will not contain any semi-join produced
      fanout, i.e. tables within SJM-SCAN(...) will not contribute to
      the cardinality of the join output.  Extra fanout produced by
      SJM-SCAN(...) will be 'absorbed' into fanout produced by ot1 ...  otN.

      The simple way to model this is to remove SJM-SCAN(...) fanout once
      we reach the point #2.
    */
    if (pos->sjm_scan_need_tables && emb_sj_nest != nullptr &&
        emb_sj_nest !=
            join->positions[pos->sjm_scan_last_inner].table->emb_sj_nest)
      /*
        Prevent that inner tables of different semijoin nests are
        interleaved for MatScan.
      */
      pos->sjm_scan_need_tables = 0;
    else {
      pos->sjm_scan_need_tables = emb_sj_nest->sj_inner_tables |
                                  emb_sj_nest->nested_join->sj_depends_on;
      pos->sjm_scan_last_inner = idx;
      Opt_trace_object(trace)
          .add_alnum("strategy", "MaterializeScan")
          .add_alnum("choice", "deferred");
    }
  } else if (sjm_strategy == SJ_OPT_MATERIALIZE_LOOKUP) {
    // Calculate access paths and cost for MaterializeLookup strategy
    double cost, rowcount;
    semijoin_mat_lookup_access_paths(idx, emb_sj_nest, &rowcount, &cost);

    Opt_trace_object trace_one_strategy(trace);
    trace_one_strategy.add_alnum("strategy", "MaterializeLookup")
        .add("cost", cost)
        .add("rows", rowcount)
        .add("duplicate_tables_left", pos->dups_producing_tables != 0);
    if (cost < best_cost || pos->dups_producing_tables) {
      /*
        NOTE: When we pick to use SJM[-Scan] we don't memcpy its POSITION
        elements to join->positions as that makes it hard to return things
        back when making one step back in join optimization. That's done
        after the QEP has been chosen.
      */
      sj_strategy = SJ_OPT_MATERIALIZE_LOOKUP;
      best_cost = cost;
      best_rowcount = rowcount;
      pos->dups_producing_tables &= ~emb_sj_nest->sj_inner_tables;
    }
    trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_MATERIALIZE_LOOKUP);
  }

  /* MaterializeScan second phase check */
  /*
    The optimizer does not support that we have inner tables from more
    than one semi-join nest within the table range.
  */
  if (pos->sjm_scan_need_tables && emb_sj_nest != nullptr &&
      emb_sj_nest !=
          join->positions[pos->sjm_scan_last_inner].table->emb_sj_nest)
    pos->sjm_scan_need_tables = 0;

  if (pos->sjm_scan_need_tables && /* Have SJM-Scan prefix */
      !(pos->sjm_scan_need_tables & remaining_tables)) {
    TABLE_LIST *const sjm_nest =
        join->positions[pos->sjm_scan_last_inner].table->emb_sj_nest;

    double cost, rowcount;

    Opt_trace_object trace_one_strategy(trace);
    trace_one_strategy.add_alnum("strategy", "MaterializeScan");

    semijoin_mat_scan_access_paths(pos->sjm_scan_last_inner, idx,
                                   remaining_tables, sjm_nest, &rowcount,
                                   &cost);
    trace_one_strategy.add("cost", cost)
        .add("rows", rowcount)
        .add("duplicate_tables_left", pos->dups_producing_tables != 0);
    /*
      Use the strategy if
       * it is cheaper then what we've had, or
       * we haven't picked any other semi-join strategy yet
      In the second case, we pick this strategy unconditionally because
      comparing cost without semi-join duplicate removal with cost with
      duplicate removal is not an apples-to-apples comparison.
    */
    if (cost < best_cost || pos->dups_producing_tables) {
      sj_strategy = SJ_OPT_MATERIALIZE_SCAN;
      best_cost = cost;
      best_rowcount = rowcount;
      pos->dups_producing_tables &= ~sjm_nest->sj_inner_tables;
    }
    trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_MATERIALIZE_SCAN);
  }

  /* Duplicate Weedout strategy handler */
  {
    /*
       Duplicate weedout can be applied after all ON-correlated and
       correlated
    */
    if (emb_sj_nest) {
      if (!pos->dupsweedout_tables) pos->first_dupsweedout_table = idx;

      pos->dupsweedout_tables |= emb_sj_nest->sj_inner_tables |
                                 emb_sj_nest->nested_join->sj_depends_on;
    }

    if (pos->dupsweedout_tables &&
        !(remaining_tables & pos->dupsweedout_tables)) {
      Opt_trace_object trace_one_strategy(trace);
      trace_one_strategy.add_alnum("strategy", "DuplicatesWeedout");
      /*
        Ok, reached a state where we could put a dups weedout point.
        Walk back and calculate
          - the join cost (this is needed as the accumulated cost may assume
            some other duplicate elimination method)
          - extra fanout that will be removed by duplicate elimination
          - duplicate elimination cost
        There are two cases:
          1. We have other strategy/ies to remove all of the duplicates.
          2. We don't.

        We need to calculate the cost in case #2 also because we need to make
        choice between this join order and others.
      */
      double rowcount, cost;
      semijoin_dupsweedout_access_paths(pos->first_dupsweedout_table, idx,
                                        &rowcount, &cost);
      /*
        Use the strategy if
         * it is cheaper then what we've had, and strategy is enabled, or
         * we haven't picked any other semi-join strategy yet
        The second part is necessary because this strategy is the last one
        to consider (it needs "the most" tables in the prefix) and we can't
        leave duplicate-producing tables not handled by any strategy.
      */
      trace_one_strategy.add("cost", cost)
          .add("rows", rowcount)
          .add("duplicate_tables_left", pos->dups_producing_tables != 0);
      if ((cost < best_cost &&
           join->positions[pos->first_dupsweedout_table]
                   .table->emb_sj_nest->nested_join->sj_enabled_strategies &
               OPTIMIZER_SWITCH_DUPSWEEDOUT) ||
          pos->dups_producing_tables) {
        sj_strategy = SJ_OPT_DUPS_WEEDOUT;
        best_cost = cost;
        best_rowcount = rowcount;
        /*
          Note, dupsweedout_tables contains inner and outer tables, even though
          "dups_producing_tables" are always inner table. Ok for this use.
        */
        pos->dups_producing_tables &= ~pos->dupsweedout_tables;
      }
      trace_one_strategy.add("chosen", sj_strategy == SJ_OPT_DUPS_WEEDOUT);
    }
  }
  pos->sj_strategy = sj_strategy;
  /*
    If a semi-join strategy is chosen, update cost and rowcount in positions
    as well. These values may be used as prefix cost and rowcount for later
    semi-join calculations, e.g for plans like "ot1 - it1 - it2 - ot2",
    where we have two semi-join nests containing it1 and it2, respectively,
    and we have a dependency between ot1 and it1, and between ot2 and it2.
    When looking at a semi-join plan for "it2 - ot2", the correct prefix cost
   (located in the join_tab for it1) must be filled in properly.

    Tables in a semijoin range, except the last in range, won't have their
    prefix_costs changed below; this is normal: when we process them, this is
    a regular join so regular costs calculated in best_ext...() are ok;
    duplicates elimination happens only at the last table in range, so it
    makes sense to correct prefix_costs of that last table.
  */
  if (sj_strategy != SJ_OPT_NONE)
    pos->set_prefix_cost(best_cost, best_rowcount);
}


// Source: sql_planner.cc
// Lines 4056-4550
