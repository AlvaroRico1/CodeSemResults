int STDCALL mysql_ping(MYSQL *mysql) {
  int res;
  DBUG_TRACE;
  res = simple_command(mysql, COM_PING, nullptr, 0, 0);
  if (res == CR_SERVER_LOST && mysql->reconnect)
    res = simple_command(mysql, COM_PING, nullptr, 0, 0);
  return res;
}


// Source: libmysql.cc
// Lines 878-885
