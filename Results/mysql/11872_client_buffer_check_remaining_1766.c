inline bool buffer_check_remaining(MYSQL *mysql, uchar *packet,
                                   ulong packet_length, size_t bytes) {
  size_t remaining_bytes;
  /* Check to avoid underflow */
  if (packet_length < (ulong)(packet - mysql->net.read_pos)) {
    set_mysql_error(mysql, CR_MALFORMED_PACKET, unknown_sqlstate);
    return false;
  }
  remaining_bytes = packet_length - (packet - mysql->net.read_pos);
  if (remaining_bytes < bytes) {
    set_mysql_error(mysql, CR_MALFORMED_PACKET, unknown_sqlstate);
    return false;
  }
  return true;
}


// Source: client.cc
// Lines 709-723
