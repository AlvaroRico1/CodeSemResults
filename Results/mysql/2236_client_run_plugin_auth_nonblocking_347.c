mysql_state_machine_status run_plugin_auth_nonblocking(MYSQL *mysql, char *data,
                                                       uint data_len,
                                                       const char *data_plugin,
                                                       const char *db) {
  DBUG_TRACE;
  mysql_async_auth *ctx = ASYNC_DATA(mysql)->connect_context->auth_context;
  if (!ctx) {
    ctx = static_cast<mysql_async_auth *>(
        my_malloc(key_memory_MYSQL, sizeof(*ctx), MYF(MY_WME | MY_ZEROFILL)));

    ctx->mysql = mysql;
    ctx->data = data;
    ctx->data_len = data_len;
    ctx->data_plugin = data_plugin;
    ctx->db = db;
    ctx->non_blocking = true;
    ctx->state_function = authsm_begin_plugin_auth;
    ASYNC_DATA(mysql)->connect_context->auth_context = ctx;
  }

  mysql_state_machine_status ret = ctx->state_function(ctx);
  if (ret == STATE_MACHINE_FAILED || ret == STATE_MACHINE_DONE) {
    my_free(ctx);
    ASYNC_DATA(mysql)->connect_context->auth_context = nullptr;
  }

  return ret;
}


// Source: client.cc
// Lines 5288-5315
