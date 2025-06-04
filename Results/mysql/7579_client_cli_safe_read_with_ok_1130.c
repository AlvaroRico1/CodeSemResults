ulong cli_safe_read_with_ok(MYSQL *mysql, bool parse_ok, bool *is_data_packet) {
  DBUG_TRACE;
  NET *net = &mysql->net;
  ulong len = 0;

  MYSQL_TRACE(READ_PACKET, mysql, ());

  if (is_data_packet) *is_data_packet = false;

  if (net->vio != nullptr) len = my_net_read(net);
  return cli_safe_read_with_ok_complete(mysql, parse_ok, is_data_packet, len);
}


// Source: client.cc
// Lines 1133-1144
