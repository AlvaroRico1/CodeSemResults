bool net_flush(NET *net) {
  bool error = false;
  DBUG_TRACE;
  if (net->buff != net->write_pos) {
    error =
        net_write_packet(net, net->buff, (size_t)(net->write_pos - net->buff));
    net->write_pos = net->buff;
  }
  /* Sync packet number if using compression */
  if (net->compress) net->pkt_nr = net->compress_pkt_nr;
  return error;
}


// Source: net_serv.cc
// Lines 281-292
