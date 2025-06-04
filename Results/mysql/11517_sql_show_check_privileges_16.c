bool Sql_cmd_show_databases::check_privileges(THD *thd) {
  TABLE_LIST *const table = thd->lex->query_tables;

  if (check_table_access(thd, SELECT_ACL, table, false, UINT_MAX, false))
    return true;

  return (specialflag & SPECIAL_SKIP_SHOW_DB) &&
         check_global_access(thd, SHOW_DB_ACL);
}


// Source: sql_show.cc
// Lines 438-446
