int STDCALL mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query,
                               ulong length) {
  MYSQL *mysql = stmt->mysql;
  DBUG_TRACE;

  if (!mysql) {
    /* mysql can be reset in mysql_close called from mysql_reconnect */
    set_stmt_error(stmt, CR_SERVER_LOST, unknown_sqlstate, nullptr);
    return 1;
  }

  /*
    Reset the last error in any case: that would clear the statement
    if the previous prepare failed.
  */
  stmt->last_errno = 0;
  stmt->last_error[0] = '\0';

  if ((int)stmt->state > (int)MYSQL_STMT_INIT_DONE) {
    /* This is second prepare with another statement */
    uchar buff[MYSQL_STMT_HEADER]; /* 4 bytes - stmt id */

    if (reset_stmt_handle(stmt, RESET_LONG_DATA | RESET_STORE_RESULT)) return 1;
    /*
      These members must be reset for API to
      function in case of error or misuse.
    */
    stmt->bind_param_done = false;
    stmt->bind_result_done = false;
    stmt->param_count = stmt->field_count = 0;
    free_root(stmt->mem_root, MYF(MY_KEEP_PREALLOC));
    free_root(&stmt->extension->fields_mem_root, MYF(0));

    int4store(buff, stmt->stmt_id);

    /*
      Close statement in server

      If there was a 'use' result from another statement, or from
      mysql_use_result it won't be freed in mysql_stmt_free_result and
      we should get 'Commands out of sync' here.
    */
    stmt->state = MYSQL_STMT_INIT_DONE;
    if (stmt_command(mysql, COM_STMT_CLOSE, buff, 4, stmt)) {
      set_stmt_errmsg(stmt, &mysql->net);
      return 1;
    }
  }

  if (stmt_command(mysql, COM_STMT_PREPARE, (const uchar *)query, length,
                   stmt)) {
    set_stmt_errmsg(stmt, &mysql->net);
    return 1;
  }

  if ((*mysql->methods->read_prepare_result)(mysql, stmt)) {
    set_stmt_errmsg(stmt, &mysql->net);
    return 1;
  }

  /*
    alloc_root will return valid address even in case when param_count
    and field_count are zero. Thus we should never rely on stmt->bind
    or stmt->params when checking for existence of placeholders or
    result set.
  */
  if (!(stmt->params = (MYSQL_BIND *)stmt->mem_root->Alloc(
            sizeof(MYSQL_BIND) * (stmt->param_count + stmt->field_count)))) {
    set_stmt_error(stmt, CR_OUT_OF_MEMORY, unknown_sqlstate, nullptr);
    return 1;
  }
  stmt->bind = stmt->params + stmt->param_count;
  stmt->state = MYSQL_STMT_PREPARE_DONE;
  DBUG_PRINT("info", ("Parameter count: %u", stmt->param_count));
  return 0;
}


// Source: libmysql.cc
// Lines 1439-1514
