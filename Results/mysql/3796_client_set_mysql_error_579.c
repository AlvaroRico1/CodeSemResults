void set_mysql_error(MYSQL *mysql, int errcode, const char *sqlstate) {
  NET *net;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("error :%d '%s'", errcode, ER_CLIENT(errcode)));
  assert(mysql != nullptr);

  if (mysql) {
    net = &mysql->net;
    net->last_errno = errcode;
    my_stpcpy(net->last_error, ER_CLIENT(errcode));
    my_stpcpy(net->sqlstate, sqlstate);
    MYSQL_TRACE(ERROR, mysql, ());
  } else {
    mysql_server_last_errno = errcode;
    my_stpcpy(mysql_server_last_error, ER_CLIENT(errcode));
  }
}


// Source: client.cc
// Lines 298-314
