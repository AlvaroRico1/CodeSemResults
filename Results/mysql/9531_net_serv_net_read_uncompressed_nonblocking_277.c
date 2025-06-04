static net_async_status net_read_uncompressed_nonblocking(NET *net,
                                                          ulong *len_ptr) {
  DBUG_TRACE;
  assert(!net->compress);
  ulong &len = *len_ptr;

  // Maintain the local states
  static net_async_status status = NET_ASYNC_COMPLETE;
  static ulong save_pos;
  static ulong total_length;

  // Initialize the states
  if (status == NET_ASYNC_COMPLETE) {
    save_pos = net->where_b;
    total_length = 0;
  }

  status = net_read_packet_nonblocking(net, &len);
  total_length += len;
  net->where_b += len;

  if (len == MAX_PACKET_LENGTH) status = NET_ASYNC_NOT_READY;
  if (status == NET_ASYNC_NOT_READY) return status;

  // Update the offsets
  net->where_b = save_pos;
  len = total_length;
  net->read_pos = net->buff + net->where_b;
  status = NET_ASYNC_COMPLETE;
  return status;
}


// Source: net_serv.cc
// Lines 1997-2027
