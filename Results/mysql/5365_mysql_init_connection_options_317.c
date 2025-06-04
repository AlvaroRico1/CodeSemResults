static bool init_connection_options(MYSQL *mysql) {
  bool handle_expired = (opt_connect_expired_password || !status.batch);

  if (opt_init_command)
    mysql_options(mysql, MYSQL_INIT_COMMAND, opt_init_command);

  if (opt_connect_timeout) {
    uint timeout = opt_connect_timeout;
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&timeout);
  }


// Source: mysql.cc
// Lines 4622-4631
