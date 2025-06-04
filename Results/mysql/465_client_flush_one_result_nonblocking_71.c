static net_async_status flush_one_result_nonblocking(MYSQL *mysql, bool *res) {
  DBUG_TRACE;

  *res = false;
  while (true) {
    ulong packet_length;
    bool is_data_packet;
    if (cli_safe_read_nonblocking(mysql, &is_data_packet, &packet_length) ==
        NET_ASYNC_NOT_READY) {
      return NET_ASYNC_NOT_READY;
    }
    mysql->packet_length = packet_length;
    if (packet_length == packet_error) {
      *res = true;
      break;
    }
    if (mysql->net.read_pos[0] != 0 && !is_data_packet) {
      if (protocol_41(mysql)) {
        uchar *pos = mysql->net.read_pos + 1;
        if (mysql->server_capabilities & CLIENT_DEPRECATE_EOF &&
            !is_data_packet) {
          read_ok_ex(mysql, packet_length);
        } else {
          mysql->warning_count = uint2korr(pos);
          pos += 2;
          mysql->server_status = uint2korr(pos);
        }
        pos += 2;
      }


// Source: client.cc
// Lines 1592-1620
