int STDCALL mysql_select_db(MYSQL *mysql, const char *db) {
  int error;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("db: '%s'", db));

  if ((error = simple_command(mysql, COM_INIT_DB, (const uchar *)db,
                              (ulong)strlen(db), 0)))
    return error;
  my_free(mysql->db);
  mysql->db = my_strdup(key_memory_MYSQL, db, MYF(MY_WME));
  return 0;
}


// Source: client.cc
// Lines 6853-6864
