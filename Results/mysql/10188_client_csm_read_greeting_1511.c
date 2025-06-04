static mysql_state_machine_status csm_read_greeting(mysql_async_connect *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  DBUG_PRINT("info", ("Read first packet."));

  if (!ctx->non_blocking)
    ctx->pkt_length = cli_safe_read(mysql, nullptr);
  else {
    if (cli_safe_read_nonblocking(mysql, nullptr, &ctx->pkt_length) ==
        NET_ASYNC_NOT_READY) {
      return STATE_MACHINE_WOULD_BLOCK;
    }
  }
  if (ctx->pkt_length == packet_error) {
    if (mysql->net.last_errno == CR_SERVER_LOST)
      set_mysql_extended_error(mysql, CR_SERVER_LOST, unknown_sqlstate,
                               ER_CLIENT(CR_SERVER_LOST_EXTENDED),
                               "reading initial communication packet",
                               socket_errno);
    return STATE_MACHINE_FAILED;
  }
  ctx->state_function = csm_parse_handshake;
  return STATE_MACHINE_CONTINUE;
}


// Source: client.cc
// Lines 6248-6271
