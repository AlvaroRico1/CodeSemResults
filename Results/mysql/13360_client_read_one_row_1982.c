static int read_one_row(MYSQL *mysql, uint fields, MYSQL_ROW row,
                        ulong *lengths) {
  ulong pkt_len;
  bool is_data_packet;

  if ((pkt_len = cli_safe_read(mysql, &is_data_packet)) == packet_error)
    return -1;

  return read_one_row_complete(mysql, pkt_len, is_data_packet, fields, row,
                               lengths);
}


// Source: client.cc
// Lines 3099-3109
