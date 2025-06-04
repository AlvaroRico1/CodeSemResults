static int normalize_dbname(const char *line, char *buff, uint buff_size) {
  MYSQL_RES *res = nullptr;

  /* Send the "USE db" commmand to the server. */
  if (mysql_query(&mysql, line)) return 1;

  /*
    Now, get the normalized database name and store it
    into the buff.
  */
  if (!mysql_query(&mysql, "SELECT DATABASE()") &&
      (res = mysql_use_result(&mysql))) {
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) {
      size_t len = strlen(row[0]);
      /* Make sure there is enough room to store the dbname. */
      if ((len > buff_size) || !memcpy(buff, row[0], len)) {
        mysql_free_result(res);
        return 1;
      }
    }
    mysql_free_result(res);
  }


// Source: mysql.cc
// Lines 4359-4381
