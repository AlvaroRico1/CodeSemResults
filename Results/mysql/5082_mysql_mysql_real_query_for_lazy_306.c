static int mysql_real_query_for_lazy(const char *buf, size_t length,
                                     bool set_params = false) {
  int error = 0;
  for (uint retry = 0;; retry++) {
    error = 0;

    if (set_params && global_attrs->set_params(&mysql)) break;
    if (!mysql_real_query(&mysql, buf, (ulong)length)) break;
    error = put_error(&mysql);
    if ((mysql_errno(&mysql) != CR_SERVER_GONE_ERROR &&
         mysql_errno(&mysql) != CR_SERVER_LOST &&
         mysql.net.error != NET_ERROR_SOCKET_UNUSABLE) ||
        retry > 1 || !opt_reconnect)
      break;
    if (reconnect()) break;
  }
  if (set_params) global_attrs->clear(connected ? &mysql : nullptr);
  return error;
}


// Source: mysql.cc
// Lines 3081-3099
