void STDCALL mysql_close(MYSQL *mysql) {
  DBUG_TRACE;
  if (mysql) /* Some simple safety */
  {
    /* If connection is still up, send a QUIT message */
    if (mysql->net.vio != nullptr &&
        mysql->net.last_errno != NET_ERROR_SOCKET_UNUSABLE &&
        mysql->net.last_errno != NET_ERROR_SOCKET_NOT_WRITABLE) {
      free_old_query(mysql);
      mysql->status = MYSQL_STATUS_READY; /* Force command */
      bool old_reconnect = mysql->reconnect;
      mysql->reconnect = false;  // avoid recursion
      if (vio_is_blocking(mysql->net.vio)) {
        simple_command(mysql, COM_QUIT, (uchar *)nullptr, 0, 1);
      } else {
        /*
          Best effort; try to toss a command on the wire, but we can't wait
          to hear back.
        */
        bool err; /* unused */
        simple_command_nonblocking(mysql, COM_QUIT, (uchar *)nullptr, 0, 1,
                                   &err);
      }
      mysql->reconnect = old_reconnect;
      end_server(mysql); /* Sets mysql->net.vio= 0 */
    }


// Source: client.cc
// Lines 7010-7035
