static ha_rows get_quick_record_count(THD *thd, JOIN_TAB *tab, ha_rows limit) {
  DBUG_TRACE;
  uchar buff[STACK_BUFF_ALLOC];
  if (check_stack_overrun(thd, STACK_MIN_SIZE, buff))
    return 0;  // Fatal error flag is set
  TABLE_LIST *const tl = tab->table_ref;
  tab->set_skip_records_in_range(
      check_skip_records_in_range_qualification(tab, thd));

  // Derived tables aren't filled yet, so no stats are available.
  if (!tl->uses_materialization()) {
    QUICK_SELECT_I *qck;
    Key_map keys_to_use = tab->const_keys;
    keys_to_use.merge(tab->skip_scan_keys);
    int error = test_quick_select(
        thd, keys_to_use,
        0,  // empty table_map
        limit,
        false,  // don't force quick range
        ORDER_NOT_RELEVANT, tab,
        tab->join_cond() ? tab->join_cond() : tab->join()->where_cond,
        &tab->needed_reg, &qck, tab->table()->force_index,
        tab->join()->query_block);
    tab->set_quick(qck);

    if (error == 1) return qck->records;
    if (error == -1) {
      tl->table->reginfo.impossible_range = true;
      return 0;
    }
    DBUG_PRINT("warning", ("Couldn't use record count on const keypart"));
  } else if (tl->is_table_function() || tl->materializable_is_const()) {
    tl->fetch_number_of_rows();
    return tl->table->file->stats.records;
  }
  return HA_POS_ERROR;
}


// Source: sql_optimizer.cc
// Lines 5858-5894
