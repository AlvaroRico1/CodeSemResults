net_async_status my_net_read_nonblocking(NET *net, ulong *len_ptr) {
  net_async_status status;
  if (net->compress)
    status = net_read_compressed_nonblocking(net, len_ptr);
  else
    status = net_read_uncompressed_nonblocking(net, len_ptr);

  if (status == NET_ASYNC_NOT_READY) return status;

  status = NET_ASYNC_COMPLETE;
  if (*len_ptr == packet_error) return status;

  DBUG_PRINT("info", ("chunk nb read: %lu", *len_ptr));
  return status;
}


// Source: net_serv.cc
// Lines 2099-2113
