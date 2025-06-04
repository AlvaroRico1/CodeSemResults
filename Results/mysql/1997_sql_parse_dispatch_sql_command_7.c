void dispatch_sql_command(THD *thd, Parser_state *parser_state) {
  DBUG_TRACE;
  DBUG_PRINT("dispatch_sql_command", ("query: '%s'", thd->query().str));

  DBUG_EXECUTE_IF("parser_debug", turn_parser_debug_on(););

  mysql_reset_thd_for_next_command(thd);
  lex_start(thd);

  thd->m_parser_state = parser_state;
  invoke_pre_parse_rewrite_plugins(thd);
  thd->m_parser_state = nullptr;

  // we produce digest if it's not explicitly turned off
  // by setting maximum digest length to zero
  if (get_max_digest_length() != 0)
    parser_state->m_input.m_compute_digest = true;

  LEX *lex = thd->lex;
  const char *found_semicolon = nullptr;

  bool err = thd->get_stmt_da()->is_error();

  if (!err) {
    err = parse_sql(thd, parser_state, nullptr);
    if (!err) err = invoke_post_parse_rewrite_plugins(thd, false);

    found_semicolon = parser_state->m_lip.found_semicolon;
  }

  DEBUG_SYNC_C("sql_parse_before_rewrite");

  if (!err) {
    /*
      Rewrite the query for logging and for the Performance Schema
      statement tables. (Raw logging happened earlier.)

      Sub-routines of mysql_rewrite_query() should try to only rewrite when
      necessary (e.g. not do password obfuscation when query contains no
      password).

      If rewriting does not happen here, thd->m_rewritten_query is still
      empty from being reset in alloc_query().
    */
    if (thd->rewritten_query().length() == 0) mysql_rewrite_query(thd);

    if (thd->rewritten_query().length()) {
      lex->safe_to_cache_query = false;  // see comments below

      thd->set_query_for_display(thd->rewritten_query().ptr(),
                                 thd->rewritten_query().length());
    } else if (thd->slave_thread) {
      /*
        In the slave, we add the information to pfs.events_statements_history,
        but not to pfs.threads, as that is what the test suite expects.
      */
      MYSQL_SET_STATEMENT_TEXT(thd->m_statement_psi, thd->query().str,
                               thd->query().length);
    } else {
      thd->set_query_for_display(thd->query().str, thd->query().length);
    }

    if (!(opt_general_log_raw || thd->slave_thread)) {
      if (thd->rewritten_query().length())
        query_logger.general_log_write(thd, COM_QUERY,
                                       thd->rewritten_query().ptr(),
                                       thd->rewritten_query().length());
      else {
        size_t qlen = found_semicolon ? (found_semicolon - thd->query().str)
                                      : thd->query().length;

        query_logger.general_log_write(thd, COM_QUERY, thd->query().str, qlen);
      }
    }
  }

  DEBUG_SYNC_C("sql_parse_after_rewrite");

  if (!err) {
    thd->m_statement_psi = MYSQL_REFINE_STATEMENT(
        thd->m_statement_psi, sql_statement_info[thd->lex->sql_command].m_key);

    if (mqh_used && thd->get_user_connect() &&
        check_mqh(thd, lex->sql_command)) {
      if (thd->is_classic_protocol())
        thd->get_protocol_classic()->get_net()->error = NET_ERROR_UNSET;
    } else {
      if (!thd->is_error()) {
        /*
          Binlog logs a string starting from thd->query and having length
          thd->query_length; so we set thd->query_length correctly (to not
          log several statements in one event, when we executed only first).
          We set it to not see the ';' (otherwise it would get into binlog
          and Query_log_event::print() would give ';;' output).
          This also helps display only the current query in SHOW
          PROCESSLIST.
        */
        if (found_semicolon && (ulong)(found_semicolon - thd->query().str))
          thd->set_query(
              thd->query().str,
              static_cast<size_t>(found_semicolon - thd->query().str - 1));
        /* Actually execute the query */
        if (found_semicolon) {
          lex->safe_to_cache_query = false;
          thd->server_status |= SERVER_MORE_RESULTS_EXISTS;
        }
        lex->set_trg_event_type_for_tables();

        int error MY_ATTRIBUTE((unused));
        if (unlikely(thd->security_context()->password_expired() &&
                     lex->sql_command != SQLCOM_SET_PASSWORD &&
                     lex->sql_command != SQLCOM_SET_OPTION &&
                     lex->sql_command != SQLCOM_ALTER_USER)) {
          my_error(ER_MUST_CHANGE_PASSWORD, MYF(0));
          error = 1;
        } else {
          resourcegroups::Resource_group *src_res_grp = nullptr;
          resourcegroups::Resource_group *dest_res_grp = nullptr;
          MDL_ticket *ticket = nullptr;
          MDL_ticket *cur_ticket = nullptr;
          auto mgr_ptr = resourcegroups::Resource_group_mgr::instance();
          bool switched = mgr_ptr->switch_resource_group_if_needed(
              thd, &src_res_grp, &dest_res_grp, &ticket, &cur_ticket);

          error = mysql_execute_command(thd, true);

          if (switched)
            mgr_ptr->restore_original_resource_group(thd, src_res_grp,
                                                     dest_res_grp);
          thd->resource_group_ctx()->m_switch_resource_group_str[0] = '\0';
          if (ticket != nullptr)
            mgr_ptr->release_shared_mdl_for_resource_group(thd, ticket);
          if (cur_ticket != nullptr)
            mgr_ptr->release_shared_mdl_for_resource_group(thd, cur_ticket);
        }
      }
    }
  } else {
    /*
      Log the failed raw query in the Performance Schema. This statement did not
      parse, so there is no way to tell if it may contain a password of not.

      The tradeoff is:
        a) If we do log the query, a user typing by accident a broken query
           containing a password will have the password exposed. This is very
           unlikely, and this behavior can be documented. Remediation is to use
           a new password when retyping the corrected query.

        b) If we do not log the query, finding broken queries in the client
           application will be much more difficult. This is much more likely.

      Considering that broken queries can typically be generated by attempts at
      SQL injection, finding the source of the SQL injection is critical, so the
      design choice is to log the query text of broken queries (a).
    */
    thd->set_query_for_display(thd->query().str, thd->query().length);

    /* Instrument this broken statement as "statement/sql/error" */
    thd->m_statement_psi = MYSQL_REFINE_STATEMENT(
        thd->m_statement_psi, sql_statement_info[SQLCOM_END].m_key);

    assert(thd->is_error());
    DBUG_PRINT("info",
               ("Command aborted. Fatal_error: %d", thd->is_fatal_error()));
  }

  THD_STAGE_INFO(thd, stage_freeing_items);
  sp_cache_enforce_limit(thd->sp_proc_cache, stored_program_cache_size);
  sp_cache_enforce_limit(thd->sp_func_cache, stored_program_cache_size);
  thd->lex->destroy();
  thd->end_statement();
  thd->cleanup_after_query();
  assert(thd->change_list.is_empty());

  DEBUG_SYNC(thd, "query_rewritten");
}


// Source: sql_parse.cc
// Lines 4876-5051
