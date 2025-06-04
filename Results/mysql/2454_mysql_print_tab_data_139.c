static void print_tab_data(MYSQL_RES *result) {
  MYSQL_ROW cur;
  MYSQL_FIELD *field;
  ulong *lengths;

  if (opt_silent < 2 && column_names) {
    int first = 0;
    while ((field = mysql_fetch_field(result))) {
      if (first++) (void)tee_fputs("\t", PAGER);
      (void)tee_fputs(field->name, PAGER);
    }
    (void)tee_fputs("\n", PAGER);
  }


// Source: mysql.cc
// Lines 3938-3950
