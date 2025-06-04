bool Optimize_table_order::semijoin_loosescan_fill_driving_table_position(
    const JOIN_TAB *tab, table_map remaining_tables, uint idx,
    double prefix_rowcount, POSITION *pos) {
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_ls(trace, "searching_loose_scan_index");

  TABLE *const table = tab->table();
  assert(remaining_tables & tab->table_ref->map());

  const ulonglong bound_sj_equalities =
      get_bound_sj_equalities(tab, excluded_tables | remaining_tables);

  // Use of quick select is a special case. Some of its properties:
  bool quick_uses_applicable_index = false;
  uint quick_max_keypart = 0;

  pos->read_cost = DBL_MAX;
  pos->use_join_buffer = false;
  /*
    No join buffer, so no need to manage any
    Deps_of_remaining_lateral_derived_tables object.
    As this function calculates some read cost, we have to include any lateral
    materialization cost:
  */
  double derived_mat_cost =
      (tab->table_ref->is_derived() &&
       tab->table_ref->derived_query_expression()->m_lateral_deps)
          ? lateral_derived_cost(tab, idx, prefix_rowcount, join->cost_model())
          : 0;

  Opt_trace_array trace_all_idx(trace, "indexes");

  /*
    For each index, we calculate how many key segments of this index
    we can use.
  */
  for (Key_use *keyuse = tab->keyuse(); keyuse->table_ref == tab->table_ref;) {
    const uint key = keyuse->key;

    Key_use *const start_key = keyuse;
    Opt_trace_object trace_idx(trace);
    trace_idx.add_utf8("index", table->key_info[key].name);

    /*
      Equalities where one comparand is in index and other comparand is a
      not-yet-available expression.
    */
    ulonglong handled_sj_equalities = 0;
    key_part_map handled_keyparts = 0;
    /*
      Biggest index (starting at 0) of keyparts used for the "handled", not
      "bound", equalities.
    */
    uint max_keypart = 0;

    // For each keypart
    while (keyuse->table_ref == tab->table_ref && keyuse->key == key) {
      const uint keypart = keyuse->keypart;
      // For each way to access the keypart
      for (; keyuse->table_ref == tab->table_ref && keyuse->key == key &&
             keyuse->keypart == keypart;
           ++keyuse) {
        /*
          If this Key_use is not about a semi-join equality, or references an
          excluded table, or does not reference a not-yet-available table, or
          is for fulltext, or is over a prefix, then it is not a "handled sj
          equality".
        */
        if ((keyuse->sj_pred_no == UINT_MAX) ||
            (excluded_tables & keyuse->used_tables) ||
            !(remaining_tables & keyuse->used_tables) ||
            (keypart == FT_KEYPART) ||
            (table->key_info[key].key_part[keypart].key_part_flag &
             HA_PART_KEY_SEG))
          continue;
        handled_sj_equalities |= 1ULL << keyuse->sj_pred_no;
        handled_keyparts |= keyuse->keypart_map;
        assert(max_keypart <= keypart);  // see sort_keyuse()
        max_keypart = keypart;
      }
    }

    const key_part_map bound_keyparts = start_key->bound_keyparts;

    /*
      We can use semi-join LooseScan if duplicate elimination is going to work
      for all semi-join equalities. Duplicate elimination:
      - works for a bound semi-join equality, because this equality is tested
      before the nested loop leaves the last inner table of this semi-join
      nest.
      - works for a handled semi-join equality thanks to key comparison; key
      comparison works if:
        * the handled key parts are over a full field (not a prefix, otherwise
        two values, differing only after the prefix, would be treated as
        duplicates)
        * and any key part before the handled key parts, is bound (same
        justification as for "works for a bound semi-join equality" above).

      That gives us these requirements:
      1. All IN-equalities are either bound or handled.
      2. No hole in sequence of key parts.

      An example where (2) matters:
        SELECT * FROM ot1
        WHERE a IN (SELECT it1.b FROM it1 JOIN it2 ON it1.a = it2.a).
      Say the plan is it1-ot1-it2 and it1 has an index on (a,b). The semi-join
      equality is handled, by the second key part (it1.b). But the first key
      part is not bound (it2.a is not available). So there is a hole. If the
      rows of it1 are, in index order: (X,Z),(Y,Z), then the key comparison
      will let both rows pass; after joining with ot1 this will duplicate
      any row of ot1 having ot1.a=Z.

      We add this third requirement:
      3. At least one IN-equality is handled.
      In theory it is a superfluous restriction. Consider:
        select * from t2 as t3, t2
        where t2.b=t3.b and
              (t2.b) in (select b*3 from t1 where a=10);
      If the plan is t3-t1-t2, and we are looking at an index on t1.a:
      bound_sj_equalities==1 (because outer expression is equal to t3.b which
      is available), handled_sj_equalities==0 (no index on 'b*3'),
      handled_keyparts==0, bound_keyparts==1 (t1.a=10).
      We could set up 'ref' on t1.a (=10), with a "LooseScan key comparison
      length" (join_tab->loosescan_key_len) of size(t1.a), and a condition on
      t1 (t1->m_condition) of "t1.b*3=t3.b". After finding a match in t2
      (t2->m_condition="t2.b=t3.b"), the key comparison would skip all other
      rows of t1 returned by ref access. But this is a bit degenerate,
      FirstMatch-like.
    */
    if ((handled_sj_equalities | bound_sj_equalities) !=  // (1)
        LOWER_BITS(
            ulonglong,
            tab->emb_sj_nest->nested_join->sj_inner_exprs.size()))  // (1)
    {
      trace_idx.add("index_handles_needed_semijoin_equalities", false);
      continue;
    }
    if (handled_keyparts == 0)  // (3)
    {
      trace_idx.add("some_index_part_used", false);
      continue;
    }
    if ((LOWER_BITS(key_part_map, max_keypart + 1) &  // (2)
         ~(bound_keyparts | handled_keyparts)) != 0)  // (2)
    {
      trace_idx.add("index_can_remove_duplicates", false);
      continue;
    }

    // Ok, can use the strategy

    if (tab->quick() && tab->quick()->index == key &&
        tab->quick()->get_type() == QUICK_SELECT_I::QS_TYPE_RANGE) {
      quick_uses_applicable_index = true;
      quick_max_keypart = max_keypart;
    }

    if (bound_keyparts & 1) {
      Opt_trace_object trace_ref(trace, "ref");
      trace_ref.add("cost", start_key->read_cost);
      if (start_key->read_cost < pos->read_cost) {
        // @TODO use rec-per-key-based fanout calculations
        pos->loosescan_key = key;
        pos->read_cost = start_key->read_cost;
        pos->rows_fetched = start_key->fanout;
        pos->loosescan_parts = max_keypart + 1;
        pos->key = start_key;
        trace_ref.add("chosen", true);
      }
    } else if (tab->table()->covering_keys.is_set(key)) {
      /*
        There are no usable bound IN-equalities, e.g. we have

        outer_expr IN (SELECT innertbl.key FROM ...)

        and outer_expr cannot be evaluated yet, so it's actually full
        index scan and not a ref access
      */
      Opt_trace_object trace_cov_scan(trace, "covering_scan");

      // Calculate the cost of complete loose index scan.
      double rowcount = rows2double(tab->table()->file->stats.records);

      // The cost is entire index scan cost
      const double cost =
          tab->table()->file->index_scan_cost(key, 1, rowcount).total_cost();

      /*
        Now find out how many different keys we will get (for now we
        ignore the fact that we have "keypart_i=const" restriction for
        some key components, that may make us think that loose
        scan will produce more distinct records than it actually will)
      */
      if (tab->table()->key_info[key].has_records_per_key(max_keypart)) {
        const rec_per_key_t rpc =
            tab->table()->key_info[key].records_per_key(max_keypart);
        rowcount = rowcount / rpc;
      }

      trace_cov_scan.add("cost", cost);
      // @TODO: previous version also did /2
      if (cost < pos->read_cost) {
        pos->loosescan_key = key;
        pos->read_cost = cost;
        pos->rows_fetched = rowcount;
        pos->loosescan_parts = max_keypart + 1;
        pos->key = nullptr;
        trace_cov_scan.add("chosen", true);
      }
    } else
      trace_idx.add("ref_possible", false).add("covering_scan_possible", false);

  }  // ... for (Key_use *keyuse=tab->keyuse(); etc

  trace_all_idx.end();

  if (quick_uses_applicable_index && idx == join->const_tables) {
    Opt_trace_object trace_range(trace, "range_scan");
    trace_range.add("cost", tab->quick()->cost_est);
    // @TODO: this the right part restriction:
    if (tab->quick()->cost_est.total_cost() < pos->read_cost) {
      pos->loosescan_key = tab->quick()->index;
      pos->read_cost = tab->quick()->cost_est.total_cost();
      // this is ok because idx == join->const_tables
      pos->rows_fetched = rows2double(tab->quick()->records);
      pos->loosescan_parts = quick_max_keypart + 1;
      pos->key = nullptr;
      trace_range.add("chosen", true);
    }
  }

  if (pos->read_cost != DBL_MAX) {
    pos->read_cost += derived_mat_cost;
    pos->filter_effect = calculate_condition_filter(
        tab, pos->key, ~remaining_tables & ~excluded_tables, pos->rows_fetched,
        false, false, trace_ls);
    return true;
  }

  return false;

  // @todo need ref_depend_map ?
}


// Source: sql_planner.cc
// Lines 1570-1813
