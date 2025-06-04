uint build_bitmap_for_nested_joins(mem_root_deque<TABLE_LIST *> *join_list,
                                   uint first_unused) {
  DBUG_TRACE;
  for (TABLE_LIST *table : *join_list) {
    NESTED_JOIN *nested_join;
    if ((nested_join = table->nested_join)) {
      // We should have a join condition or a semi-join condition or both
      assert((table->join_cond() != nullptr) || table->is_sj_nest());

      nested_join->nj_map = 0;
      nested_join->nj_total = 0;
      /*
        We only record nested join information for outer join nests.
        Tables belonging in semi-join nests are recorded in the
        embedding outer join nest, if one exists.
      */
      if (table->join_cond()) {
        assert(first_unused < sizeof(nested_join_map) * 8);
        nested_join->nj_map = (nested_join_map)1 << first_unused++;
        nested_join->nj_total = nested_join->join_list.size();
      } else if (table->is_sj_nest()) {
        NESTED_JOIN *const outer_nest =
            table->embedding ? table->embedding->nested_join : nullptr;
        /*
          The semi-join nest has already been counted into the table count
          for the outer join nest as one table, so subtract 1 from the
          table count.
        */
        if (outer_nest)
          outer_nest->nj_total += (nested_join->join_list.size() - 1);
      } else
        assert(false);

      first_unused =
          build_bitmap_for_nested_joins(&nested_join->join_list, first_unused);
    }
  }


// Source: sql_optimizer.cc
// Lines 4695-4731
