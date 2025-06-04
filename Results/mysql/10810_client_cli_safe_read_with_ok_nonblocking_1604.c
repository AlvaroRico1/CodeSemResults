net_async_status cli_safe_read_with_ok_nonblocking(MYSQL *mysql, bool parse_ok,
                                                   bool *is_data_packet,
                                                   ulong *res) {
  NET *net = &mysql->net;
  ulong len = 0;
  DBUG_TRACE;
  assert(net->vio);

  if (NET_ASYNC_NOT_READY == my_net_read_nonblocking(net, &len)) {
    return NET_ASYNC_NOT_READY;
  }

  DBUG_PRINT("info",
             ("total nb read: %lu,  net->where_b: %lu", len, net->where_b));

  *res = cli_safe_read_with_ok_complete(mysql, parse_ok, is_data_packet, len);

  /*
    In case, packet is too large or connection is lost, net_end() is called to
    free up net->extention. Thus return NET_ASYNC_ERROR.
  */
  if ((*res == packet_error) && (NET_ASYNC_DATA(net) == nullptr)) {
    return NET_ASYNC_ERROR;
  }
  return NET_ASYNC_COMPLETE;
}


// Source: client.cc
// Lines 1081-1106
