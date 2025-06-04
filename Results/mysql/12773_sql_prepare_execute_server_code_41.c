bool Execute_sql_statement::execute_server_code(THD *thd) {
  sql_digest_state *parent_digest;
  PSI_statement_locker *parent_locker;
  bool error;

  if (alloc_query(thd, m_sql_text.str, m_sql_text.length)) return true;

  Parser_state parser_state;
  if (parser_state.init(thd, thd->query().str, thd->query().length))
    return true;

  parser_state.m_lip.multi_statements = false;
  lex_start(thd);

  parent_digest = thd->m_digest;
  parent_locker = thd->m_statement_psi;
  thd->m_digest = nullptr;
  thd->m_statement_psi = nullptr;
  error = parse_sql(thd, &parser_state, nullptr) || thd->is_error();
  thd->m_digest = parent_digest;
  thd->m_statement_psi = parent_locker;

  if (error) goto end;

  thd->lex->set_trg_event_type_for_tables();

  parent_locker = thd->m_statement_psi;
  thd->m_statement_psi = nullptr;

  /*
    Rewrite first (if needed); execution might replace passwords
    with hashes in situ without flagging it, and then we'd make
    a hash of that hash.
  */
  rewrite_query_if_needed(thd);
  log_execute_line(thd);

  error = mysql_execute_command(thd);
  thd->m_statement_psi = parent_locker;

end:
  lex_end(thd->lex);

  return error;
}

}  // namespace


// Source: sql_prepare.cc
// Lines 2225-2271
