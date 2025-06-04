ulong my_net_read(NET *net) {
  size_t len;
  /* turn off non blocking operations */
  if (!vio_is_blocking(net->vio)) vio_set_blocking_flag(net->vio, true);

  if (net->compress)
    net_read_compressed_packet(net, len);
  else
    net_read_uncompressed_packet(net, len);

  return static_cast<ulong>(len);
}


// Source: net_serv.cc
// Lines 2195-2206
