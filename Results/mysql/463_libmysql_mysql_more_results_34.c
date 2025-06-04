bool STDCALL mysql_more_results(MYSQL *mysql) {
  bool res;
  DBUG_TRACE;

  res = ((mysql->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0);
  DBUG_PRINT("exit", ("More results exists ? %d", res));
  return res;
}


// Source: libmysql.cc
// Lines 4284-4291
