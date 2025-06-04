void my_net_local_init(NET *net) {
  ulong local_net_buffer_length = 0;
  ulong local_max_allowed_packet = 0;

  (void)mysql_get_option(nullptr, MYSQL_OPT_MAX_ALLOWED_PACKET,
                         &local_max_allowed_packet);
  (void)mysql_get_option(nullptr, MYSQL_OPT_NET_BUFFER_LENGTH,
                         &local_net_buffer_length);

  net->max_packet = (uint)local_net_buffer_length;
  my_net_set_read_timeout(net, CLIENT_NET_READ_TIMEOUT);
  my_net_set_write_timeout(net, CLIENT_NET_WRITE_TIMEOUT);
  my_net_set_retry_count(net, CLIENT_NET_RETRY_COUNT);
  net->max_packet_size =
      std::max(local_net_buffer_length, local_max_allowed_packet);
}


// Source: libmysql.cc
// Lines 995-1010
