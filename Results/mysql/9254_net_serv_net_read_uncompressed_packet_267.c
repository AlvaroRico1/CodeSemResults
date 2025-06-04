static void net_read_uncompressed_packet(NET *net, size_t &len) {
  size_t complen;
  assert(!net->compress);
  len = net_read_packet(net, &complen);
  if (len == MAX_PACKET_LENGTH) {
    /* First packet of a multi-packet.  Concatenate the packets */
    ulong save_pos = net->where_b;
    size_t total_length = 0;
    do {
      net->where_b += len;
      total_length += len;
      len = net_read_packet(net, &complen);
    } while (len == MAX_PACKET_LENGTH);
    if (len != packet_error) len += total_length;
    net->where_b = save_pos;
  }


// Source: net_serv.cc
// Lines 2124-2139
