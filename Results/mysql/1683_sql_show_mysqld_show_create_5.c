bool mysqld_show_create(THD *thd, TABLE_LIST *table_list) {
  Protocol *protocol = thd->get_protocol();
  char buff[2048];
  mem_root_deque<Item *> field_list(thd->mem_root);
  String buffer(buff, sizeof(buff), system_charset_info);
  bool error = true;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("db: %s  table: %s", table_list->db, table_list->table_name));

  /*
    Metadata locks taken during SHOW CREATE should be released when
    the statmement completes as it is an information statement.
  */
  MDL_savepoint mdl_savepoint = thd->mdl_context.mdl_savepoint();

  /* We want to preserve the tree for views. */
  thd->lex->context_analysis_only |= CONTEXT_ANALYSIS_ONLY_VIEW;

  {
    /*
      If there is an error during processing of an underlying view, an
      error message is wanted, but it has to be converted to a warning,
      so that execution can continue.
      This is handled by the Show_create_error_handler class.

      Use open_tables() instead of open_tables_for_query(). If an error occurs,
      this will ensure that tables are not closed on error, but remain open
      for the rest of the processing of the SHOW statement.
    */
    Show_create_error_handler view_error_suppressor(thd, table_list);
    thd->push_internal_handler(&view_error_suppressor);

    Prepared_stmt_arena_holder ps_arena_holder(thd);
    uint counter;
    bool open_error = open_tables(thd, &table_list, &counter,
                                  MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL);
    if (!open_error && table_list->is_view_or_derived() &&
        !table_list->derived_query_expression()->is_prepared()) {
      /*
        Prepare result table for view so that we can read the column list.
        Notice that Show_create_error_handler remains active, so that any
        errors due to missing underlying objects are converted to warnings.
      */
      open_error = table_list->resolve_derived(thd, true);

      /*
        If a suppressed error occurred, the query expression may not be
        marked as prepared. Mark it as prepared nevertheless, this is
        good enough for the SHOW CREATE command, as we only need the
        preparation for errors against referenced objects.
      */
      if (!table_list->derived_query_expression()->is_prepared()) {
        table_list->derived_query_expression()->set_prepared();
      }
    }
    thd->pop_internal_handler();
    if (open_error && (thd->killed || thd->is_error())) goto exit;

    /*
      Table_function::print() only works after the table function has been
      resolved. If resolving the view fails, and the view references an
      unresolved table function, raise an error instead of calling print() on
      the unresolved table function.
    */
    if (open_error && table_list->is_view()) {
      for (TABLE_LIST *tl = table_list; tl != nullptr; tl = tl->next_global) {
        if (tl->is_table_function() && tl->table == nullptr) {
          my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                   "SHOW CREATE VIEW on a view that references a non-existent "
                   "table and a table function.");
          goto exit;
        }
      }
    }


// Source: sql_show.cc
// Lines 1093-1167
