static int semijoin_order_allows_materialization(const JOIN *join,
                                                 table_map remaining_tables,
                                                 const JOIN_TAB *tab,
                                                 uint idx) {
  assert(!(remaining_tables & tab->table_ref->map()));
  /*
   Check if
    1. We're in a semi-join nest that can be run with SJ-materialization
    2. All the tables from the subquery are in the prefix
  */
  const TABLE_LIST *emb_sj_nest = tab->emb_sj_nest;
  if (!emb_sj_nest || !emb_sj_nest->nested_join->sjm.positions ||
      (remaining_tables & emb_sj_nest->sj_inner_tables))
    return SJ_OPT_NONE;

  /*
    Walk back and check if all immediately preceding tables are from
    this semi-join.
  */
  const uint n_tables = my_count_bits(emb_sj_nest->sj_inner_tables);
  for (uint i = 1; i < n_tables; i++) {
    if (join->positions[idx - i].table->emb_sj_nest != emb_sj_nest)
      return SJ_OPT_NONE;
  }

  /*
    Must use MaterializeScan strategy if there are outer correlated tables
    among the remaining tables, otherwise, if possible, use MaterializeLookup.
  */
  if ((remaining_tables & emb_sj_nest->nested_join->sj_depends_on) ||
      !emb_sj_nest->nested_join->sjm.lookup_allowed) {
    if (emb_sj_nest->nested_join->sjm.scan_allowed)
      return SJ_OPT_MATERIALIZE_SCAN;
    return SJ_OPT_NONE;
  }
  return SJ_OPT_MATERIALIZE_LOOKUP;
}


// Source: sql_planner.cc
// Lines 2168-2204
