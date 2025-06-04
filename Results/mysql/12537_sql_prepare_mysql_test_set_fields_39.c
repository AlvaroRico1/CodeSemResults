static bool mysql_test_set_fields(Prepared_statement *stmt, TABLE_LIST *tables,
                                  List<set_var_base> *var_list) {
  List_iterator_fast<set_var_base> it(*var_list);
  THD *thd = stmt->thd;
  set_var_base *var;
  DBUG_TRACE;
  assert(stmt->m_arena.is_stmt_prepare());

  if (tables &&
      check_table_access(thd, SELECT_ACL, tables, false, UINT_MAX, false))
    return true; /* purecov: inspected */

  if (open_tables_for_query(thd, tables, MYSQL_OPEN_FORCE_SHARED_MDL))
    return true; /* purecov: inspected */

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  while ((var = it++)) {
    if (var->light_check(thd)) return true; /* purecov: inspected */
    var->cleanup();
  }

  thd->lex->unit->set_prepared();
  thd->lex->save_cmd_properties(thd);
  thd->lex->cleanup(thd, false);

  return false;
}


// Source: sql_prepare.cc
// Lines 1063-1090
