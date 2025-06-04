net_async_status STDCALL mysql_next_result_nonblocking(MYSQL *mysql) {
  DBUG_TRACE;
  net_async_status status;
  if (mysql->status != MYSQL_STATUS_READY) {
    set_mysql_error(mysql, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate);
    return NET_ASYNC_ERROR;
  }
  net_clear_error(&mysql->net);
  mysql->affected_rows = ~(uint64_t)0;

  if (mysql->server_status & SERVER_MORE_RESULTS_EXISTS) {
    status = (*mysql->methods->next_result_nonblocking)(mysql);
    return status;
  } else {
    MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);
  }

  return NET_ASYNC_COMPLETE_NO_MORE_RESULTS; /* No more results */
}


// Source: libmysql.cc
// Lines 4332-4350
