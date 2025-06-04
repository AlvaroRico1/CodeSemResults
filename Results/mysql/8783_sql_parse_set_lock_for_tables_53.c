void Query_block::set_lock_for_tables(thr_lock_type lock_type) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("lock_type: %d  for_update: %d", lock_type,
                       lock_type >= TL_READ_NO_INSERT));
  for (TABLE_LIST *table = table_list.first; table; table = table->next_local)
    set_lock_for_table({lock_type, THR_WAIT}, table);
}


// Source: sql_parse.cc
// Lines 5970-5976
