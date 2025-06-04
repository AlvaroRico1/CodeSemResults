static mysql_state_machine_status authsm_handle_change_user_result(
    mysql_async_auth *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  DBUG_PRINT("info", ("OK packet length=%lu", ctx->pkt_length));
  if (ctx->pkt_length == packet_error) {
    if (mysql->net.last_errno == CR_SERVER_LOST)
      set_mysql_extended_error(mysql, CR_SERVER_LOST, unknown_sqlstate,
                               ER_CLIENT(CR_SERVER_LOST_EXTENDED),
                               "reading authorization packet", errno);
    return STATE_MACHINE_FAILED;
  }

  if (mysql->net.read_pos[0] == 254) {
    ctx->state_function = authsm_run_second_authenticate_user;
  } else
    ctx->state_function = authsm_finish_auth;

  return STATE_MACHINE_CONTINUE;
}


// Source: client.cc
// Lines 5470-5489
