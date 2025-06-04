bool stmt_causes_implicit_commit(const THD *thd, uint mask) {
  DBUG_TRACE;
  const LEX *lex = thd->lex;

  if ((sql_command_flags[lex->sql_command] & mask) == 0 ||
      thd->is_plugin_fake_ddl())
    return false;

  switch (lex->sql_command) {
    case SQLCOM_DROP_TABLE:
      return !lex->drop_temporary;
    case SQLCOM_ALTER_TABLE:
    case SQLCOM_CREATE_TABLE:
      /* If CREATE TABLE of non-temporary table or without
        START TRANSACTION, do implicit commit */
      return (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE ||
              lex->create_info->m_transactional_ddl) == 0;
    case SQLCOM_SET_OPTION:
      /* Implicitly commit a transaction started by a SET statement */
      return lex->autocommit;
    case SQLCOM_RESET:
      return lex->option_type != OPT_PERSIST;
    default:
      return true;
  }
}


// Source: sql_parse.cc
// Lines 361-386
