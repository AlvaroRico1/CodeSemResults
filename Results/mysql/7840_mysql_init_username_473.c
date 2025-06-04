static void init_username() {
  my_free(full_username);
  my_free(part_username);

  MYSQL_RES *result = nullptr;
  if (!mysql_query(&mysql, "select USER()") &&
      (result = mysql_use_result(&mysql))) {
    MYSQL_ROW cur = mysql_fetch_row(result);
    full_username = my_strdup(PSI_NOT_INSTRUMENTED, cur[0], MYF(MY_WME));
    part_username =
        my_strdup(PSI_NOT_INSTRUMENTED, strtok(cur[0], "@"), MYF(MY_WME));
    (void)mysql_fetch_row(result);  // Read eof
  }


// Source: mysql.cc
// Lines 5259-5271
