void mysql_stmt_get_longdata(THD *thd, Prepared_statement *stmt,
                             uint param_number, uchar *str, ulong length) {
  DBUG_TRACE;

  thd->status_var.com_stmt_send_long_data++;
  Diagnostics_area new_stmt_da(false);
  thd->push_diagnostics_area(&new_stmt_da);

  Item_param *param = stmt->param_array[param_number];
  param->set_longdata((char *)str, length);
  if (thd->get_stmt_da()->is_error()) {
    stmt->m_arena.set_state(Query_arena::STMT_ERROR);
    stmt->last_errno = thd->get_stmt_da()->mysql_errno();
    snprintf(stmt->last_error, sizeof(stmt->last_error), "%.*s",
             MYSQL_ERRMSG_SIZE - 1, thd->get_stmt_da()->message_text());
  }
  thd->pop_diagnostics_area();

  query_logger.general_log_print(thd, thd->get_command(), NullS);
}


// Source: sql_prepare.cc
// Lines 2093-2112
