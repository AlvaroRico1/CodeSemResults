int STDCALL mysql_stmt_next_result(MYSQL_STMT *stmt) {
  MYSQL *mysql = stmt->mysql;
  int rc;
  DBUG_TRACE;

  if (!mysql) return 1;

  if (stmt->last_errno) return stmt->last_errno;

  if (mysql->server_status & SERVER_MORE_RESULTS_EXISTS) {
    if (reset_stmt_handle(stmt, RESET_STORE_RESULT)) return 1;
  }

  rc = mysql_next_result(mysql);

  if (rc) {
    set_stmt_errmsg(stmt, &mysql->net);
    return rc;
  }

  if (mysql->status == MYSQL_STATUS_GET_RESULT)
    mysql->status = MYSQL_STATUS_STATEMENT_GET_RESULT;

  stmt->state = MYSQL_STMT_EXECUTE_DONE;
  stmt->bind_result_done = false;
  stmt->field_count = mysql->field_count;

  if (mysql->field_count) {
    alloc_stmt_fields(stmt);
    prepare_to_fetch_result(stmt);
  }

  return 0;
}


// Source: libmysql.cc
// Lines 4352-4385
