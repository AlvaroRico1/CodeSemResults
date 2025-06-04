static net_async_status send_client_reply_packet_nonblocking(MCPVIO_EXT *mpvio,
                                                             const uchar *pkt,
                                                             int pkt_len,
                                                             bool *result) {
  DBUG_TRACE;
  MYSQL *mysql = mpvio->mysql;
  mysql_async_auth *ctx = ASYNC_DATA(mysql)->connect_context->auth_context;
  net_async_status status;

  bool error = false;
  if (!ctx->change_user_buff) {
    error =
        prep_client_reply_packet(mpvio, pkt, pkt_len, &ctx->change_user_buff,
                                 &ctx->change_user_buff_len);
    if (error) {
      goto end;
    }
  }

  status = my_net_write_nonblocking(&mysql->net, (uchar *)ctx->change_user_buff,
                                    ctx->change_user_buff_len, &error);

  if (status == NET_ASYNC_NOT_READY) {
    return NET_ASYNC_NOT_READY;
  }

end:
  *result = error;
  my_free(ctx->change_user_buff);
  ctx->change_user_buff = nullptr;

  return NET_ASYNC_COMPLETE;
}


// Source: client.cc
// Lines 4893-4925
