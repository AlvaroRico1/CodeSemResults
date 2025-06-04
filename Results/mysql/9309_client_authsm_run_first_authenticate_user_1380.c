static mysql_state_machine_status authsm_run_first_authenticate_user(
    mysql_async_auth *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  MYSQL_TRACE(AUTH_PLUGIN, mysql, (ctx->auth_plugin->name));

  if (ctx->non_blocking && ctx->auth_plugin->authenticate_user_nonblocking) {
    net_async_status status = ctx->auth_plugin->authenticate_user_nonblocking(
        (struct MYSQL_PLUGIN_VIO *)&ctx->mpvio, mysql, &ctx->res);
    if (status == NET_ASYNC_NOT_READY) {
      return STATE_MACHINE_WOULD_BLOCK;
    }
  } else {
    ctx->res = ctx->auth_plugin->authenticate_user(
        (struct MYSQL_PLUGIN_VIO *)&ctx->mpvio, mysql);
  }


// Source: client.cc
// Lines 5373-5388
