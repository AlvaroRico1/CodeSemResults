bool Sql_cmd_load_table::execute(THD *thd) {
  LEX *const lex = thd->lex;

  uint privilege =
      (lex->duplicates == DUP_REPLACE ? INSERT_ACL | DELETE_ACL : INSERT_ACL) |
      (m_is_local_file ? 0 : FILE_ACL);

  if (m_is_local_file) {
    if (!thd->get_protocol()->has_client_capability(CLIENT_LOCAL_FILES) ||
        !opt_local_infile) {
      my_error(ER_CLIENT_LOCAL_FILES_DISABLED, MYF(0));
      return true;
    }
  }

  if (check_one_table_access(thd, privilege, lex->query_tables)) return true;

  /* Push strict / ignore error handler */
  Ignore_error_handler ignore_handler;
  Strict_error_handler strict_handler;
  if (thd->lex->is_ignore())
    thd->push_internal_handler(&ignore_handler);
  else if (thd->is_strict_mode())
    thd->push_internal_handler(&strict_handler);

  bool res = execute_inner(thd, lex->duplicates);

  /* Pop ignore / strict error handler */
  if (thd->lex->is_ignore() || thd->is_strict_mode())
    thd->pop_internal_handler();

  return res;
}


// Source: sql_load.cc
// Lines 2118-2150
