int mysql_execute_command(THD *thd, bool first_level) {
  int res = false;
  LEX *const lex = thd->lex;
  /* first Query_block (have special meaning for many of non-SELECTcommands) */
  Query_block *const query_block = lex->query_block;
  /* first table of first Query_block */
  TABLE_LIST *const first_table = query_block->get_table_list();
  /* list of all tables in query */
  TABLE_LIST *all_tables;
  // keep GTID violation state in order to roll it back on statement failure
  bool gtid_consistency_violation_state = thd->has_gtid_consistency_violation;
  assert(query_block->master_query_expression() == lex->unit);
  DBUG_TRACE;
  /* EXPLAIN OTHER isn't explainable command, but can have describe flag. */
  assert(!lex->is_explain() || is_explainable_query(lex->sql_command) ||
         lex->sql_command == SQLCOM_EXPLAIN_OTHER);

  assert(!thd->m_transactional_ddl.inited() ||
         thd->in_active_multi_stmt_transaction());

  /*
    If there is a CREATE TABLE...START TRANSACTION command which
    is not yet committed or rollbacked, then we should allow only
    BINLOG INSERT, COMMIT or ROLLBACK command.
    TODO: Should we really check name of table when we cable BINLOG INSERT ?
  */
  if (thd->m_transactional_ddl.inited() && lex->sql_command != SQLCOM_COMMIT &&
      lex->sql_command != SQLCOM_ROLLBACK &&
      lex->sql_command != SQLCOM_BINLOG_BASE64_EVENT) {
    my_error(ER_STATEMENT_NOT_ALLOWED_AFTER_START_TRANSACTION, MYF(0));
    binlog_gtid_end_transaction(thd);
    return 1;
  }

  thd->work_part_info = nullptr;

  if (thd->optimizer_switch_flag(OPTIMIZER_SWITCH_SUBQUERY_TO_DERIVED))
    lex->add_statement_options(OPTION_NO_CONST_TABLES);

  /*
    Each statement or replication event which might produce deadlock
    should handle transaction rollback on its own. So by the start of
    the next statement transaction rollback request should be fulfilled
    already.
  */
  assert(!thd->transaction_rollback_request || thd->in_sub_stmt);
  /*
    In many cases first table of main Query_block have special meaning =>
    check that it is first table in global list and relink it first in
    queries_tables list if it is necessary (we need such relinking only
    for queries with subqueries in select list, in this case tables of
    subqueries will go to global list first)

    all_tables will differ from first_table only if most upper Query_block
    do not contain tables.

    Because of above in place where should be at least one table in most
    outer Query_block we have following check:
    assert(first_table == all_tables);
    assert(first_table == all_tables && first_table != 0);
  */
  lex->first_lists_tables_same();
  /* should be assigned after making first tables same */
  all_tables = lex->query_tables;
  /* set context for commands which do not use setup_tables */
  query_block->context.resolve_in_table_list_only(
      query_block->get_table_list());

  thd->get_stmt_da()->reset_diagnostics_area();
  if ((thd->lex->keep_diagnostics != DA_KEEP_PARSE_ERROR) &&
      (thd->lex->keep_diagnostics != DA_KEEP_DIAGNOSTICS)) {
    /*
      No parse errors, and it's not a diagnostic statement:
      remove the sql conditions from the DA!
      For diagnostic statements we need to keep the conditions
      around so we can inspec them.
    */
    thd->get_stmt_da()->reset_condition_info(thd);
  }

  if (thd->resource_group_ctx()->m_warn != 0) {
    auto res_grp_name = thd->resource_group_ctx()->m_switch_resource_group_str;
    switch (thd->resource_group_ctx()->m_warn) {
      case WARN_RESOURCE_GROUP_UNSUPPORTED: {
        auto res_grp_mgr = resourcegroups::Resource_group_mgr::instance();
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_FEATURE_UNSUPPORTED,
                            ER_THD(thd, ER_FEATURE_UNSUPPORTED),
                            "Resource groups", res_grp_mgr->unsupport_reason());
        break;
      }
      case WARN_RESOURCE_GROUP_UNSUPPORTED_HINT:
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_WARN_UNSUPPORTED_HINT,
                            ER_THD(thd, ER_WARN_UNSUPPORTED_HINT),
                            "Subquery or Stored procedure or Trigger");
        break;
      case WARN_RESOURCE_GROUP_TYPE_MISMATCH: {
        ulonglong pfs_thread_id = 0;
        /*
          Resource group is unsupported with DISABLE_PSI_THREAD.
          The below #ifdef is required for compilation when DISABLE_PSI_THREAD
          is enabled.
        */
#ifdef HAVE_PSI_THREAD_INTERFACE
        pfs_thread_id = PSI_THREAD_CALL(get_current_thread_internal_id)();
#endif  // HAVE_PSI_THREAD_INTERFACE
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_RESOURCE_GROUP_BIND_FAILED,
                            ER_THD(thd, ER_RESOURCE_GROUP_BIND_FAILED),
                            res_grp_name, pfs_thread_id,
                            "System resource group can't be bound"
                            " with a session thread");
        break;
      }
      case WARN_RESOURCE_GROUP_NOT_EXISTS:
        push_warning_printf(
            thd, Sql_condition::SL_WARNING, ER_RESOURCE_GROUP_NOT_EXISTS,
            ER_THD(thd, ER_RESOURCE_GROUP_NOT_EXISTS), res_grp_name);
        break;
      case WARN_RESOURCE_GROUP_ACCESS_DENIED:
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_SPECIFIC_ACCESS_DENIED_ERROR,
                            ER_THD(thd, ER_SPECIFIC_ACCESS_DENIED_ERROR),
                            "SUPER OR RESOURCE_GROUP_ADMIN OR "
                            "RESOURCE_GROUP_USER");
    }
    thd->resource_group_ctx()->m_warn = 0;
    res_grp_name[0] = '\0';
  }

  if (unlikely(thd->slave_thread)) {
    if (!check_database_filters(thd, thd->db().str, lex->sql_command)) {
      binlog_gtid_end_transaction(thd);
      return 0;
    }

    if (lex->sql_command == SQLCOM_DROP_TRIGGER) {
      /*
        When dropping a trigger, we need to load its table name
        before checking slave filter rules.
      */
      TABLE_LIST *trigger_table = nullptr;
      (void)get_table_for_trigger(thd, lex->spname->m_db, lex->spname->m_name,
                                  true, &trigger_table);
      if (trigger_table != nullptr) {
        lex->add_to_query_tables(trigger_table);
        all_tables = trigger_table;
      } else {
        /*
          If table name cannot be loaded,
          it means the trigger does not exists possibly because
          CREATE TRIGGER was previously skipped for this trigger
          according to slave filtering rules.
          Returning success without producing any errors in this case.
        */
        binlog_gtid_end_transaction(thd);
        return 0;
      }

      // force searching in slave.cc:tables_ok()
      all_tables->updating = true;
    }

    /*
      For fix of BUG#37051, the master stores the table map for update
      in the Query_log_event, and the value is assigned to
      thd->table_map_for_update before executing the update
      query.

      If thd->table_map_for_update is set, then we are
      replicating from a new master, we can use this value to apply
      filter rules without opening all the tables. However If
      thd->table_map_for_update is not set, then we are
      replicating from an old master, so we just skip this and
      continue with the old method. And of course, the bug would still
      exist for old masters.
    */
    if (lex->sql_command == SQLCOM_UPDATE_MULTI && thd->table_map_for_update) {
      table_map table_map_for_update = thd->table_map_for_update;
      uint nr = 0;
      TABLE_LIST *table;
      for (table = all_tables; table; table = table->next_global, nr++) {
        if (table_map_for_update & ((table_map)1 << nr))
          table->updating = true;
        else
          table->updating = false;
      }

      if (all_tables_not_ok(thd, all_tables)) {
        /* we warn the slave SQL thread */
        my_error(ER_SLAVE_IGNORED_TABLE, MYF(0));
        binlog_gtid_end_transaction(thd);
        return 0;
      }

      for (table = all_tables; table; table = table->next_global)
        table->updating = true;
    }


// Source: sql_parse.cc
// Lines 2706-2904
