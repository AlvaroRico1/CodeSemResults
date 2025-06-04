void set_mysql_extended_error(MYSQL *mysql, int errcode, const char *sqlstate,
                              const char *format, ...) {
  NET *net;
  va_list args;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("error :%d '%s'", errcode, format));
  assert(mysql != nullptr);

  net = &mysql->net;
  net->last_errno = errcode;
  va_start(args, format);
  vsnprintf(net->last_error, sizeof(net->last_error) - 1, format, args);
  va_end(args);
  my_stpcpy(net->sqlstate, sqlstate);

  MYSQL_TRACE(ERROR, mysql, ());
}


// Source: client.cc
// Lines 346-362
