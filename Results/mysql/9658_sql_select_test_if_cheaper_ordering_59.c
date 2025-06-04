bool test_if_cheaper_ordering(const JOIN_TAB *tab, ORDER_with_src *order,
                              TABLE *table, Key_map usable_keys, int ref_key,
                              ha_rows select_limit, int *new_key,
                              int *new_key_direction, ha_rows *new_select_limit,
                              uint *new_used_key_parts,
                              uint *saved_best_key_parts) {
  DBUG_TRACE;
  /*
    Check whether there is an index compatible with the given order
    usage of which is cheaper than usage of the ref_key index (ref_key>=0)
    or a table scan.
    It may be the case if ORDER/GROUP BY is used with LIMIT.
  */
  ha_rows best_select_limit = HA_POS_ERROR;
  JOIN *join = tab ? tab->join() : nullptr;
  if (join) ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  uint nr;
  uint best_key_parts = 0;
  int best_key_direction = 0;
  ha_rows best_records = 0;
  double read_time;
  int best_key = -1;
  bool is_best_covering = false;
  double fanout = 1;
  ha_rows table_records = table->file->stats.records;
  bool group = join && join->grouped && order == &join->group_list;
  double refkey_rows_estimate =
      static_cast<double>(table->quick_condition_rows);
  const bool has_limit = (select_limit != HA_POS_ERROR);
  const join_type cur_access_method = tab ? tab->type() : JT_ALL;

  if (join) {
    read_time = tab->position()->read_cost;
    for (uint jt = tab->idx() + 1; jt < join->primary_tables; jt++) {
      POSITION *pos = join->best_ref[jt]->position();
      fanout *= pos->rows_fetched * pos->filter_effect;
      if (fanout < 0) break;  // fanout became 'unknown'
    }
  } else
    read_time = table->file->table_scan_cost().total_cost();

  /*
    Calculate the selectivity of the ref_key for REF_ACCESS. For
    RANGE_ACCESS we use table->quick_condition_rows.
  */
  if (ref_key >= 0 && cur_access_method == JT_REF) {
    if (table->quick_keys.is_set(ref_key))
      refkey_rows_estimate = static_cast<double>(table->quick_rows[ref_key]);
    else {
      const KEY *ref_keyinfo = table->key_info + ref_key;
      if (ref_keyinfo->has_records_per_key(tab->ref().key_parts - 1))
        refkey_rows_estimate =
            ref_keyinfo->records_per_key(tab->ref().key_parts - 1);
      else
        refkey_rows_estimate = 1.0;  // No index statistics
    }


// Source: sql_select.cc
// Lines 4946-5001
