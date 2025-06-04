static mysql_state_machine_status authsm_finish_auth(mysql_async_auth *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  /*
    net->read_pos[0] should always be 0 here if the server implements
    the protocol correctly
  */
  ctx->res = (mysql->net.read_pos[0] != 0);

  MYSQL_TRACE(AUTHENTICATED, mysql, ());
  return ctx->res ? STATE_MACHINE_FAILED : STATE_MACHINE_DONE;
}


// Source: client.cc
// Lines 5568-5579
