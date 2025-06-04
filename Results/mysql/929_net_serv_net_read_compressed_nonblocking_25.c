static net_async_status net_read_compressed_nonblocking(NET *net,
                                                        ulong *len_ptr) {
  DBUG_TRACE;
  assert(net->compress);
  ulong &len = *len_ptr;

  /* Maintain the local states to read the multipacket asynchronously */
  static size_t start_of_packet;
  static size_t first_packet_offset;
  static size_t buf_length;
  static uint multi_byte_packet = 0;
  static net_async_status status = NET_ASYNC_COMPLETE;

  if (status != NET_ASYNC_NOT_READY)
    net_read_init_offsets(net, start_of_packet, first_packet_offset,
                          multi_byte_packet, buf_length);

  for (;;) {
    /*  Read the current packet in net->buff */
    if (net_read_process_buffer(net, start_of_packet, buf_length,
                                multi_byte_packet, first_packet_offset))
      break;

    /*
      Read the mysql packet from vio, uncompress it and make it accessable
      through net->buff.
    */
    status = net_read_packet_nonblocking(net, &len);
    if (status == NET_ASYNC_NOT_READY) {
      net->save_char = net->buff[first_packet_offset];
      net->buf_length = buf_length;
      return status;
    }

    if (len == packet_error) {
      status = NET_ASYNC_COMPLETE;
      return status;
    }
    buf_length += len;
  }
  /*
    Once the packets are read in the net->buff, adjust the tracking offsets to
    the appropiate values
  */
  len = net_read_update_offsets(net, start_of_packet, first_packet_offset,
                                buf_length, multi_byte_packet);
  status = NET_ASYNC_COMPLETE;
  return status;
}


// Source: net_serv.cc
// Lines 1939-1987
