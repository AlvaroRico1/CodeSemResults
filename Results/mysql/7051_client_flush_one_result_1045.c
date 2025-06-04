static bool flush_one_result(MYSQL *mysql) {
  ulong packet_length;
  bool is_data_packet;

  assert(mysql->status != MYSQL_STATUS_READY);

  do {
    packet_length = cli_safe_read(mysql, &is_data_packet);
    /*
      There is an error reading from the connection,
      or (sic!) there were no error and no
      data in the stream, i.e. no more data from the server.
      Since we know our position in the stream (somewhere in
      the middle of a result set), this latter case is an error too
      -- each result set must end with a EOF packet.
      cli_safe_read() has set an error for us, just return.
    */
    if (packet_length == packet_error) return true;
  } while (mysql->net.read_pos[0] == 0 || is_data_packet);

  /* Analyse final OK packet (EOF packet if it is old client) */

  if (protocol_41(mysql)) {
    uchar *pos = mysql->net.read_pos + 1;
    if (mysql->server_capabilities & CLIENT_DEPRECATE_EOF && !is_data_packet)
      read_ok_ex(mysql, packet_length);
    else {
      mysql->warning_count = uint2korr(pos);
      pos += 2;
      mysql->server_status = uint2korr(pos);
    }
    pos += 2;
  }


// Source: client.cc
// Lines 1636-1668
