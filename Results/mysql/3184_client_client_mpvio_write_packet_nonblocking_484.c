static net_async_status client_mpvio_write_packet_nonblocking(
    struct MYSQL_PLUGIN_VIO *mpv, const uchar *pkt, int pkt_len, int *result) {
  DBUG_TRACE;
  MCPVIO_EXT *mpvio = (MCPVIO_EXT *)mpv;
  bool error = false;

  if (mpvio->packets_written == 0) {
    /* mysql_change_user_nonblocking not implemented yet. */
    assert(!mpvio->mysql_change_user);
    net_async_status status =
        send_client_reply_packet_nonblocking(mpvio, pkt, pkt_len, &error);
    if (status == NET_ASYNC_NOT_READY) {
      return NET_ASYNC_NOT_READY;
    }
  } else {
    NET *net = &mpvio->mysql->net;

    MYSQL_TRACE(SEND_AUTH_DATA, mpvio->mysql, ((size_t)pkt_len, pkt));

    if (mpvio->mysql->thd)
      *result = 1; /* no chit-chat in embedded */
    else {
      net_async_status status =
          my_net_write_nonblocking(net, pkt, pkt_len, &error);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      *result = error;

      if (error) {
        set_mysql_extended_error(mpvio->mysql, CR_SERVER_LOST, unknown_sqlstate,
                                 ER_CLIENT(CR_SERVER_LOST_EXTENDED),
                                 "sending authentication information", errno);
      } else {
        MYSQL_TRACE(PACKET_SENT, mpvio->mysql, ((size_t)pkt_len));
      }
    }
  }
  mpvio->packets_written++;
  *result = error ? -1 : 0;
  return NET_ASYNC_COMPLETE;
}


// Source: client.cc
// Lines 5117-5158
