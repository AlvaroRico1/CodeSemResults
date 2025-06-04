float calculate_condition_filter(const JOIN_TAB *const tab,
                                 const Key_use *const keyuse,
                                 table_map used_tables, double fanout,
                                 bool is_join_buffering, bool write_to_trace,
                                 Opt_trace_object &parent_trace) {
  /*
    Because calculating condition filtering has a cost, it should only
    be done if the filter is meaningful. It is meaningful if the query
    is an EXPLAIN, or if the filter may influence the QEP.

    Note that this means that EXPLAIN FOR CONNECTION will typically
    not find a calculated filtering value for the last table in a QEP
    (i.e., it will be 1.0).

    Calculate condition filter if
    1)  Condition filtering is enabled, and
    2a) Condition filtering is about to be calculated for a scan that
        might do join buffering. Rationale: When a table is scanned
        and joined with rows in a buffer, constant predicates are
        evaluated on rows in the joined table. Only rows that pass the
        constant predicates are attempted joined with the prefix rows
        in the buffer. The filtering effect is the estimate of how
        many rows pass the constant predicate evaluation.
    2b) 'tab' is not the last table that will be added to the plan.
        Rationale: filtering only reduces the number of rows sent to
        the next step in the join ordering and therefore has no effect
        on the last table in the join order, or
    2c) 'tab' is in a subselect. Rationale: for subqueries, view/table
        materializations, the filtering effect is needed to
        estimate the number of rows in the potentially materialized
        subquery, or
    2d) 'tab' is in a query_block with a semijoin nest. Rationale: the
        cost of some of the duplicate elimination strategies depends
        on the size of the output, or
    2e) The query has either an order by or group by clause and a limit clause.
        Rationale: some of the limit optimizations take the filtering effect
        on the last table into account.
    2f) Statement is EXPLAIN

    Note: Even in the case of a single table query, the filtering
    effect may effect the QEP because the cost of sorting fewer rows
    is lower. This is currently ignored since single table
    optimization performance is so important.
  */
  const THD *thd = tab->join()->thd;
  TABLE *const table = tab->table();
  const table_map remaining_tables =
      ~used_tables & ~tab->table_ref->map() & tab->join()->all_table_map;
  if (!(thd->optimizer_switch_flag(
            OPTIMIZER_SWITCH_COND_FANOUT_FILTER) &&  // 1)
        (is_join_buffering ||                        // 2a
         remaining_tables != 0 ||                    // 2b
         tab->join()
                 ->query_block->master_query_expression()
                 ->outer_query_block() != nullptr ||     // 2c
         !tab->join()->query_block->sj_nests.empty() ||  // 2d
         ((!tab->join()->order.empty() || !tab->join()->group_list.empty()) &&
          tab->join()->query_expression()->select_limit_cnt !=
              HA_POS_ERROR) ||      // 2e
         thd->lex->is_explain())))  // 2f
    return COND_FILTER_ALLPASS;

  // No filtering is calculated if we expect less than one row to be fetched
  if (fanout < 1.0 || tab->found_records < 1.0 || tab->records() < 1.0)
    return COND_FILTER_ALLPASS;

  /*
    cond_set has the column bit set for each column involved in a
    predicate. If no bits are set, there are no predicates on this
    table.
  */
  if (bitmap_is_clear_all(&table->cond_set)) return COND_FILTER_ALLPASS;

  /*
    Use TABLE::tmp_set to keep track of fields that should not
    contribute to filtering effect.
    First, verify it's not used.
  */
  assert(bitmap_is_clear_all(&table->tmp_set));

  float filter = COND_FILTER_ALLPASS;

  Opt_trace_context *const trace = &tab->join()->thd->opt_trace;

  Opt_trace_disable_I_S disable_trace(trace, !write_to_trace);
  Opt_trace_array filtering_effect_trace(trace, "filtering_effect");

  /*
    If ref/range access, the condition is already included in the
    record estimate. The fields used by the ref/range access method
    shall not contribute to the filtering estimate since 'filter' is
    percentage of fetched rows that are filtered away.
  */
  if (keyuse) {
    const KEY *key = table->key_info + keyuse->key;

    if (keyuse[0].keypart == FT_KEYPART) {
      /*
        Fulltext indexes are special because keyuse->keypart does not
        contain the keypart number but a constant (FT_KEYPART)
        defining that it is a fulltext index. However, since fulltext
        search demands that all indexed keyparts are used, iterating
        over the next 'actual_key_parts' works.
      */
      for (uint i = 0; i < key->actual_key_parts; i++)
        bitmap_set_bit(&table->tmp_set, key->key_part[i].field->field_index());
    } else {
      const Key_use *curr_ku = keyuse;

      /*
        'keyuse' describes the chosen ref access method for 'tab'. It
        is a pointer into JOIN::keyuse_array which describes all
        possible ways to perform ref access for all indexes of all
        tables. E.g., keyuse for the index "t1.idx(kp1, kp2)" and
        query condition

          "WHERE t1.kp1=1 AND t1.kp1=t2.col AND t1.kp2=2"
        will be
          [keyuse(t1.kp1,1),keyuse(t1.kp1,t2.col),keyuse(t1.kp2,2)]

        1) Since there may be multiple ways to ref-access any index it
        is not enough to look at keyuse[0..actual_key_parts-1].
        Instead, stop iterating when curr_ku no longer points to the
        specified index in 'tab'.

        2) In addition, there may be predicates that are relevant for
        an index but that will not be used by the 'ref' access (the
        keypart is not bound). This could e.g. be because the
        predicate depends on a value from a table later in the join
        sequence or because there is ref_or_null access:

          "WHERE t1.kp1=1 AND t1.kp2=t2.col"
             => t1.kp2 not used by ref since it depends on
                table later in join sequenc
          "WHERE (t1.kp1=1 OR t1.kp1 IS NULL) AND t1.kp2=2"
             => t1.kp2 not used by ref since kp1 is ref_or_null
      */
      while (curr_ku->table_ref == tab->table_ref &&         // 1)
             curr_ku->key == keyuse->key &&                  // 1)
             curr_ku->keypart_map & keyuse->bound_keyparts)  // 2)
      {
        bitmap_set_bit(&table->tmp_set,
                       key->key_part[curr_ku->keypart].field->field_index());
        curr_ku++;
      }
    }
  } else if (tab->quick())


// Source: sql_planner.cc
// Lines 1208-1354
