static bool get_current_db() {
  MYSQL_RES *res;

  /* If one_database is set, current_db is not supposed to change. */
  if (one_database) return false;

  my_free(current_db);
  current_db = nullptr;
  /* In case of error below current_db will be NULL */
  if (!mysql_query(&mysql, "SELECT DATABASE()") &&
      (res = mysql_use_result(&mysql))) {
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0])
      current_db = my_strdup(PSI_NOT_INSTRUMENTED, row[0], MYF(MY_WME));
    mysql_free_result(res);
  } else {
    /* We failed to issue the command and we likely lost connection */
    return true;
  }


// Source: mysql.cc
// Lines 3055-3073
