bool Prepared_statement::execute_loop(String *expanded_query,
                                      bool open_cursor) {
  const int MAX_REPREPARE_ATTEMPTS = 3;
  Reprepare_observer reprepare_observer;
  bool error;
  bool reprepared_for_types MY_ATTRIBUTE((unused)) = false;
  int reprepare_attempt = 0;

  /* Check if we got an error when sending long data */
  if (m_arena.get_state() == Query_arena::STMT_ERROR) {
    my_message(last_errno, last_error, MYF(0));
    return true;
  }

  assert(!thd->get_stmt_da()->is_set());

  if (unlikely(!thd->security_context()->account_is_locked() &&
               thd->security_context()->password_expired() &&
               lex->sql_command != SQLCOM_SET_PASSWORD &&
               lex->sql_command != SQLCOM_ALTER_USER)) {
    my_error(ER_MUST_CHANGE_PASSWORD, MYF(0));
    return true;
  }

  // Remember if the general log was temporarily disabled when repreparing the
  // statement for a secondary engine.
  bool general_log_temporarily_disabled = false;

  // Reprepare statement unconditionally if it contains UDF references
  if (lex->has_udf() && reprepare()) return true;

  // Reprepare statement if protocol has changed.
  // Note: this is not possible in current code base, hence the assert.
  if (m_active_protocol != nullptr &&
      m_active_protocol != thd->get_protocol()) {
    assert(false);
    if (reprepare()) return true;
  }

reexecute:
  /*
    If the item_list is not empty, we'll wrongly free some externally
    allocated items when cleaning up after validation of the prepared
    statement.
  */
  assert(thd->item_list() == nullptr);

  if (!check_parameter_types()) {
    // Only one reprepare is required in case of parameter mismatch
    assert(!reprepared_for_types);
    reprepared_for_types = true;
    if (reprepare()) return true;
    goto reexecute;
  }

  reprepared_for_types = false;
  /*
    Install the metadata observer. If some metadata version is
    different from prepare time and an observer is installed,
    the observer method will be invoked to push an error into
    the error stack.
  */
  Reprepare_observer *stmt_reprepare_observer = nullptr;

  if (sql_command_flags[lex->sql_command] & CF_REEXECUTION_FRAGILE) {
    reprepare_observer.reset_reprepare_observer();
    stmt_reprepare_observer = &reprepare_observer;
  }

  thd->push_reprepare_observer(stmt_reprepare_observer);

  error = execute(expanded_query, open_cursor) || thd->is_error();

  thd->pop_reprepare_observer();

  // Check if we have a non-fatal error and the statement allows reexecution.
  if ((sql_command_flags[lex->sql_command] & CF_REEXECUTION_FRAGILE) && error &&
      !thd->is_fatal_error() && !thd->is_killed()) {
    // If we have an error due to a metadata change, reprepare the
    // statement and execute it again.
    if (reprepare_observer.is_invalidated()) {
      assert(thd->get_stmt_da()->mysql_errno() == ER_NEED_REPREPARE);

      if ((reprepare_attempt++ < MAX_REPREPARE_ATTEMPTS) &&
          DBUG_EVALUATE_IF("simulate_max_reprepare_attempts_hit_case", false,
                           true)) {
        thd->clear_error();
        error = reprepare();
        DEBUG_SYNC(thd, "after_statement_reprepare");
      } else {
        /*
          Reprepare_observer sets error status in DA but Sql_condition is not
          added. Please check Reprepare_observer::report_error(). Pushing
          Sql_condition for ER_NEED_REPREPARE here.
        */
        Diagnostics_area *da = thd->get_stmt_da();
        da->push_warning(thd, da->mysql_errno(), da->returned_sqlstate(),
                         Sql_condition::SL_ERROR, da->message_text());
      }
    } else {
      // Otherwise, if repreparation was requested, try again in the primary
      // or secondary engine, depending on cause.
      const uint err_seen = thd->get_stmt_da()->mysql_errno();
      if (err_seen == ER_PREPARE_FOR_SECONDARY_ENGINE ||
          (err_seen == ER_NEED_REPREPARE &&
           reprepare_attempt++ < MAX_REPREPARE_ATTEMPTS)) {
        assert((thd->secondary_engine_optimization() ==
                Secondary_engine_optimization::PRIMARY_TENTATIVELY) ||
               err_seen == ER_NEED_REPREPARE);
        assert(!lex->unit->is_executed());
        thd->clear_error();
        if (err_seen == ER_PREPARE_FOR_SECONDARY_ENGINE)
          thd->set_secondary_engine_optimization(
              Secondary_engine_optimization::SECONDARY);
        else
          thd->set_secondary_engine_optimization(
              Secondary_engine_optimization::PRIMARY_ONLY);
        // Disable the general log. The query was written to the general log in
        // the first attempt to execute it. No need to write it twice.
        general_log_temporarily_disabled |= disable_general_log(thd);
        error = reprepare();
      }

      // If (re-?)preparation or optimization failed and it was for
      // a secondary storage engine, disable the secondary storage
      // engine and try again without it.
      if (error && lex->m_sql_cmd != nullptr &&
          thd->secondary_engine_optimization() ==
              Secondary_engine_optimization::SECONDARY &&
          !lex->unit->is_executed()) {
        thd->clear_error();
        thd->set_secondary_engine_optimization(
            Secondary_engine_optimization::PRIMARY_ONLY);
        error = reprepare();
        if (!error) {
          // The reprepared statement should not use a secondary engine.
          assert(!lex->m_sql_cmd->using_secondary_storage_engine());
          lex->m_sql_cmd->disable_secondary_storage_engine();
        }
      }
    }

    if (!error) /* Success */
      goto reexecute;
  }
  reset_stmt_params(this);

  // Reenable the general log if it was temporarily disabled while repreparing
  // and executing a statement for a secondary engine.
  if (general_log_temporarily_disabled)
    thd->variables.option_bits &= ~OPTION_LOG_OFF;

  return error;
}


// Source: sql_prepare.cc
// Lines 2934-3087
