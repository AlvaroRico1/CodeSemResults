static bool opt_flush_ok_packet(MYSQL *mysql, bool *is_ok_packet) {
  bool is_data_packet;
  ulong packet_length = cli_safe_read(mysql, &is_data_packet);

  if (packet_length == packet_error) return true;

  /* cli_safe_read always reads a non-empty packet. */
  assert(packet_length);

  *is_ok_packet =
      ((mysql->net.read_pos[0] == 0) ||
       ((mysql->server_capabilities & CLIENT_DEPRECATE_EOF) &&
        mysql->net.read_pos[0] == 254 && packet_length < MAX_PACKET_LENGTH));
  if (*is_ok_packet) {
    read_ok_ex(mysql, packet_length);
#if defined(CLIENT_PROTOCOL_TRACING)
    if (mysql->server_status & SERVER_MORE_RESULTS_EXISTS)
      MYSQL_TRACE_STAGE(mysql, WAIT_FOR_RESULT);
    else
      MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);
#endif
  }

  return false;
}


// Source: client.cc
// Lines 1686-1710
