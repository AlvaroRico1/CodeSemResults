static mysql_state_machine_status authsm_read_change_user_result(
    mysql_async_auth *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  /* read the OK packet (or use the cached value in mysql->net.read_pos */
  if (ctx->res == CR_OK) {
    if (ctx->non_blocking) {
      net_async_status status =
          (*mysql->methods->read_change_user_result_nonblocking)(
              mysql, &ctx->pkt_length);
      if (status == NET_ASYNC_NOT_READY) {
        return STATE_MACHINE_WOULD_BLOCK;
      }
    } else {
      ctx->pkt_length = (*mysql->methods->read_change_user_result)(mysql);
    }


// Source: client.cc
// Lines 5444-5459
