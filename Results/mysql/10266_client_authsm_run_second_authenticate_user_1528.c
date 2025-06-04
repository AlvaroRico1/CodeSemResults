static mysql_state_machine_status authsm_run_second_authenticate_user(
    mysql_async_auth *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  /* The server asked to use a different authentication plugin */
  if (ctx->pkt_length < 2) {
    set_mysql_error(mysql, CR_MALFORMED_PACKET,
                    unknown_sqlstate); /* purecov: inspected */
    return STATE_MACHINE_FAILED;
  } else {
    /* "use different plugin" packet */
    uint len;
    ctx->auth_plugin_name = (char *)mysql->net.read_pos + 1;
    len = (uint)strlen(
        ctx->auth_plugin_name); /* safe as my_net_read always appends \0 */
    ctx->mpvio.cached_server_reply.pkt_len = ctx->pkt_length - len - 2;
    ctx->mpvio.cached_server_reply.pkt = mysql->net.read_pos + len + 2;
    DBUG_PRINT("info", ("change plugin packet from server for plugin %s",
                        ctx->auth_plugin_name));
  }


// Source: client.cc
// Lines 5495-5514
