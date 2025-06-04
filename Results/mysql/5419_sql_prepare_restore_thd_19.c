  void restore_thd(THD *thd, Prepared_statement *stmt) {
    DBUG_TRACE;

    mysql_mutex_lock(&thd->LOCK_thd_data);
    stmt->lex = thd->lex;
    thd->lex = m_lex;
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    thd->set_safe_display(m_safe_to_display);

    stmt->m_query_string = thd->query();
    thd->set_query(m_query_string);

    return;
  }

  /**
    Save the current rewritten query prior to
    rewriting the prepared statement.
  */
  void save_rlb(THD *thd) {
    DBUG_TRACE;

    if (thd->rewritten_query().length() > 0) {
      /* Duplicate the original rewritten query. */
      m_rewritten_query.copy(thd->rewritten_query());
      /* Swap the duplicate with the original. */
      thd->swap_rewritten_query(m_rewritten_query);
    }

    return;
  }

  /**
    Restore the rewritten query after the prepared
    statement has finished executing.
  */
  void restore_rlb(THD *thd) {
    DBUG_TRACE;

    if (m_rewritten_query.length() > 0) {
      /* Restore with swap() instead of '='. */
      thd->swap_rewritten_query(m_rewritten_query);
      /* Free the rewritten prepared statement. */
      m_rewritten_query.mem_free();
    }

    return;
  }
};


// Source: sql_prepare.cc
// Lines 420-469
