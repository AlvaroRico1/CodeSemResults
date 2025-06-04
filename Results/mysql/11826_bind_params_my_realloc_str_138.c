static bool my_realloc_str(NET *net, ulong length) {
  ulong buf_length = (ulong)(net->write_pos - net->buff);
  bool res = false;
  DBUG_TRACE;
  if (buf_length + length > net->max_packet) {
    res = net_realloc(net, buf_length + length);
    if (res) {
      if (net->last_errno == ER_OUT_OF_RESOURCES)
        net->last_errno = CR_OUT_OF_MEMORY;
      else if (net->last_errno == ER_NET_PACKET_TOO_LARGE)
        net->last_errno = CR_NET_PACKET_TOO_LARGE;

      my_stpcpy(net->sqlstate, unknown_sqlstate);
      my_stpcpy(net->last_error, ER_CLIENT(net->last_errno));
    }
    net->write_pos = net->buff + buf_length;
  }
  return res;
}


// Source: bind_params.cc
// Lines 56-74
