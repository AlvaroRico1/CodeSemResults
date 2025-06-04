bool show_precheck(THD *thd, LEX *lex, bool lock MY_ATTRIBUTE((unused))) {
  assert(lex->sql_command == SQLCOM_SHOW_CREATE_USER);
  TABLE_LIST *const tables = lex->query_tables;
  if (tables != nullptr) {
    if (check_table_access(thd, SELECT_ACL, tables, false, UINT_MAX, false))
      return true;
  }
  return false;
}


// Source: sql_parse.cc
// Lines 4716-4724
