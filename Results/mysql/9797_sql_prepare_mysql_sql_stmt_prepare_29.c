void mysql_sql_stmt_prepare(THD *thd) {
  LEX *lex = thd->lex;
  const LEX_CSTRING &name = lex->prepared_stmt_name;
  Prepared_statement *stmt;
  const char *query;
  size_t query_len = 0;
  DBUG_TRACE;

  if ((stmt = thd->stmt_map.find_by_name(name))) {
    /*
      If there is a statement with the same name, remove it. It is ok to
      remove old and fail to insert a new one at the same time.
    */
    if (stmt->is_in_use()) {
      my_error(ER_PS_NO_RECURSION, MYF(0));
      return;
    }

    MYSQL_DESTROY_PS(stmt->m_prepared_stmt);
    stmt->deallocate();
  }

  if (!(query = get_dynamic_sql_string(lex, &query_len)) ||
      !(stmt = new Prepared_statement(thd))) {
    return; /* out of memory */
  }

  stmt->set_sql_prepare();

  /* Set the name first, insert should know that this statement has a name */
  if (stmt->set_name(name)) {
    delete stmt;
    return;
  }

  if (thd->stmt_map.insert(stmt)) {
    /* The statement is deleted and an error is set if insert fails */
    return;
  }

  /* Create PS table entry, set query text after rewrite. */
  stmt->m_prepared_stmt =
      MYSQL_CREATE_PS(stmt, stmt->id, thd->m_statement_psi, stmt->name().str,
                      stmt->name().length, nullptr, 0);

  if (stmt->prepare(query, query_len, nullptr)) {
    /* Delete this stmt stats from PS table. */
    MYSQL_DESTROY_PS(stmt->m_prepared_stmt);
    /* Statement map deletes the statement on erase */
    thd->stmt_map.erase(stmt);
  } else {
    /* send the boolean tracker in the OK packet when
       @@session_track_state_change is set to ON */
    if (thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
            ->is_enabled())
      thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
          ->mark_as_changed(thd, nullptr);
    my_ok(thd, 0L, 0L, "Statement prepared");
  }
}


// Source: sql_prepare.cc
// Lines 1774-1833
