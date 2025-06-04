int cli_unbuffered_fetch(MYSQL *mysql, char **row) {
  ulong len = 0;
  bool is_data_packet;
  if (packet_error == (len = cli_safe_read(mysql, &is_data_packet))) {
    MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);
    return 1;
  }

  if (mysql->net.read_pos[0] != 0 && !is_data_packet) {
    /* in case of new client read the OK packet */
    if (mysql->server_capabilities & CLIENT_DEPRECATE_EOF)
      read_ok_ex(mysql, len);
    *row = nullptr;
    MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);
  } else {
    *row = (char *)(mysql->net.read_pos + 1);
  }

  return 0;
}


// Source: libmysql.cc
// Lines 3710-3729
