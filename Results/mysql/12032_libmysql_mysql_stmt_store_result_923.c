int STDCALL mysql_stmt_store_result(MYSQL_STMT *stmt) {
  MYSQL *mysql = stmt->mysql;
  MYSQL_DATA *result = &stmt->result;
  DBUG_TRACE;

  if (!mysql) {
    /* mysql can be reset in mysql_close called from mysql_reconnect */
    set_stmt_error(stmt, CR_SERVER_LOST, unknown_sqlstate, nullptr);
    return 1;
  }

  if (!stmt->field_count) return 0;

  if ((int)stmt->state < (int)MYSQL_STMT_EXECUTE_DONE) {
    set_stmt_error(stmt, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate, nullptr);
    return 1;
  }

  if (stmt->last_errno) {
    /* An attempt to use an invalid statement handle. */
    return 1;
  }

  if (mysql->status == MYSQL_STATUS_READY &&
      stmt->server_status & SERVER_STATUS_CURSOR_EXISTS) {
    /*
      Server side cursor exist, tell server to start sending the rows
    */
    NET *net = &mysql->net;
    uchar buff[4 /* statement id */ + 4 /* number of rows to fetch */];

    /* Send row request to the server */
    int4store(buff, stmt->stmt_id);
    int4store(buff + 4, (int)~0); /* number of rows to fetch */
    if (cli_advanced_command(mysql, COM_STMT_FETCH, buff, sizeof(buff),
                             (uchar *)nullptr, 0, true, stmt)) {
      /*
        Don't set stmt error if stmt->mysql is NULL, as the error in this case
        has already been set by mysql_prune_stmt_list().
      */
      if (stmt->mysql) set_stmt_errmsg(stmt, net);
      return 1;
    }
  } else if (mysql->status != MYSQL_STATUS_STATEMENT_GET_RESULT) {
    set_stmt_error(stmt, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate, nullptr);
    return 1;
  }


// Source: libmysql.cc
// Lines 3918-3964
