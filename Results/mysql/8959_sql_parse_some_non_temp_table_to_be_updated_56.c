bool some_non_temp_table_to_be_updated(THD *thd, TABLE_LIST *tables) {
  for (TABLE_LIST *table = tables; table; table = table->next_global) {
    assert(table->db && table->table_name);
    /*
      Update on performance_schema and temp tables are allowed
      in readonly mode.
    */
    if (table->updating && !find_temporary_table(thd, table) &&
        !is_perfschema_db(table->db, table->db_length))
      return true;
  }
  return false;
}


// Source: sql_parse.cc
// Lines 336-348
