static int get_vio_connect_timeout(MYSQL *mysql) {
  int timeout_ms;
  uint timeout_sec;

  /*
    A timeout of 0 means no timeout. Also, the connect_timeout
    option value is in seconds, while VIO timeouts are measured
    in milliseconds. Hence, check for a possible overflow. In
    case of overflow, set to no timeout.
  */
  timeout_sec = mysql->options.connect_timeout;

  if (!timeout_sec || (timeout_sec > INT_MAX / 1000))
    timeout_ms = -1;
  else
    timeout_ms = (int)(timeout_sec * 1000);

  return timeout_ms;
}


// Source: client.cc
// Lines 233-251
