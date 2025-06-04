bool mysql_rename_tables(THD *thd, TABLE_LIST *table_list) {
  TABLE_LIST *ren_table = nullptr;
  DBUG_TRACE;

  mysql_ha_rm_tables(thd, table_list);

  /*
    The below Auto_releaser allows to keep uncommitted versions of data-
    dictionary objects cached in the Dictionary_client for the whole duration
    of the statement.
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (query_logger.is_log_table_enabled(QUERY_LOG_GENERAL) ||
      query_logger.is_log_table_enabled(QUERY_LOG_SLOW)) {
    int to_table;
    const char *rename_log_table[2] = {nullptr, nullptr};

    /*
      Rules for rename of a log table:

      IF   1. Log tables are enabled
      AND  2. Rename operates on the log table and nothing is being
              renamed to the log table.
      DO   3. Throw an error message.
      ELSE 4. Perform rename.
    */

    for (to_table = 0, ren_table = table_list; ren_table;
         to_table = 1 - to_table, ren_table = ren_table->next_local) {
      int log_table_rename = 0;

      if ((log_table_rename =
               query_logger.check_if_log_table(ren_table, true))) {
        /*
          as we use log_table_rename as an array index, we need it to start
          with 0, while QUERY_LOG_SLOW == 1 and QUERY_LOG_GENERAL == 2.
          So, we shift the value to start with 0;
        */
        log_table_rename--;
        if (rename_log_table[log_table_rename]) {
          if (to_table)
            rename_log_table[log_table_rename] = nullptr;
          else {
            /*
              Two renames of "log_table TO" w/o rename "TO log_table" in
              between.
            */
            my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), ren_table->table_name,
                     ren_table->table_name);
            return true;
          }
        } else {
          if (to_table) {
            /*
              Attempt to rename a table TO log_table w/o renaming
              log_table TO some table.
            */
            my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), ren_table->table_name,
                     ren_table->table_name);
            return true;
          } else {
            /* save the name of the log table to report an error */
            rename_log_table[log_table_rename] = ren_table->table_name;
          }
        }
      }
    }
    if (rename_log_table[0] || rename_log_table[1]) {
      if (rename_log_table[0])
        my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), rename_log_table[0],
                 rename_log_table[0]);
      else
        my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), rename_log_table[1],
                 rename_log_table[1]);
      return true;
    }
  }


// Source: sql_rename.cc
// Lines 155-232
