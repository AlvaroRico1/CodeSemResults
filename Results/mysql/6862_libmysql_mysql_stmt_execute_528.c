int STDCALL mysql_stmt_execute(MYSQL_STMT *stmt) {
  MYSQL *mysql = stmt->mysql;
  DBUG_TRACE;

  if (!mysql) {
    /* Error is already set in mysql_detatch_stmt_list */
    return 1;
  }

  if (reset_stmt_handle(stmt, RESET_STORE_RESULT | RESET_CLEAR_ERROR)) return 1;
  /*
    No need to check for stmt->state: if the statement wasn't
    prepared we'll get 'unknown statement handler' error from server.
  */
  if (mysql->methods->stmt_execute(stmt)) return 1;
  stmt->state = MYSQL_STMT_EXECUTE_DONE;
  if (mysql->field_count) {
    reinit_result_set_metadata(stmt);
    prepare_to_fetch_result(stmt);
  }
  return stmt->last_errno != 0;
}


// Source: libmysql.cc
// Lines 2185-2206
