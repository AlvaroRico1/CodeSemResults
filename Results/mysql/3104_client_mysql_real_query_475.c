int STDCALL mysql_real_query(MYSQL *mysql, const char *query, ulong length) {
  int retval;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("handle: %p", mysql));
  DBUG_PRINT("query", ("Query = '%-.*s'", (int)length, query));
  DBUG_EXECUTE_IF("inject_ER_NET_READ_INTERRUPTED", {
    mysql->net.last_errno = ER_NET_READ_INTERRUPTED;
    DBUG_SET("-d,inject_ER_NET_READ_INTERRUPTED");
    return 1;
  });

  if (mysql_send_query(mysql, query, length)) return 1;
  retval = (int)(*mysql->methods->read_query_result)(mysql);
  mysql_extension_bind_free(MYSQL_EXTENSION_PTR(mysql));
  return retval;
}


// Source: client.cc
// Lines 7411-7426
