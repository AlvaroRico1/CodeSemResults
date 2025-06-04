int run_plugin_auth(MYSQL *mysql, char *data, uint data_len,
                    const char *data_plugin, const char *db) {
  DBUG_TRACE;
  mysql_state_machine_status status;
  mysql_async_auth ctx;
  memset(&ctx, 0, sizeof(ctx));

  ctx.mysql = mysql;
  ctx.data = data;
  ctx.data_len = data_len;
  ctx.data_plugin = data_plugin;
  ctx.db = db;
  ctx.non_blocking = false;
  ctx.state_function = authsm_begin_plugin_auth;

  do {
    status = ctx.state_function(&ctx);
    DBUG_PRINT("info", ("status %d", (int)status));
  } while (status != STATE_MACHINE_FAILED && status != STATE_MACHINE_DONE);

  return status == STATE_MACHINE_FAILED;
}


// Source: client.cc
// Lines 5246-5267
