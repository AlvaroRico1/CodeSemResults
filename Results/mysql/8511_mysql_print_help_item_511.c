static void print_help_item(MYSQL_ROW *cur, int num_name, int num_cat,
                            char *last_char) {
  char ccat = (*cur)[num_cat][0];
  if (*last_char != ccat) {
    put_info(ccat == 'Y' ? "categories:" : "topics:", INFO_INFO);
    *last_char = ccat;
  }
  tee_fprintf(PAGER, "   %s\n", (*cur)[num_name]);
}


// Source: mysql.cc
// Lines 3108-3116
