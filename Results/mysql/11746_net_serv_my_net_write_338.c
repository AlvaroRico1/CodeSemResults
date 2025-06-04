bool my_net_write(NET *net, const uchar *packet, size_t len) {
  uchar buff[NET_HEADER_SIZE];

  DBUG_DUMP("net write", packet, len);

  if (unlikely(!net->vio)) /* nowhere to write */
    return false;

  DBUG_EXECUTE_IF("simulate_net_write_failure", {
    my_error(ER_NET_ERROR_ON_WRITE, MYF(0));
    return 1;
  };);

  /* turn off non blocking operations */
  if (!vio_is_blocking(net->vio)) vio_set_blocking_flag(net->vio, true);
  /*
    Big packets are handled by splitting them in packets of MAX_PACKET_LENGTH
    length. The last packet is always a packet that is < MAX_PACKET_LENGTH.
    (The last packet may even have a length of 0)
  */
  while (len >= MAX_PACKET_LENGTH) {
    const ulong z_size = MAX_PACKET_LENGTH;
    int3store(buff, z_size);
    buff[3] = (uchar)net->pkt_nr++;
    if (net_write_buff(net, buff, NET_HEADER_SIZE) ||
        net_write_buff(net, packet, z_size)) {
      return true;
    }
    packet += z_size;
    len -= z_size;
  }


// Source: net_serv.cc
// Lines 433-463
