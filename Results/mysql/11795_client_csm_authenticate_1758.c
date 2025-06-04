static mysql_state_machine_status csm_authenticate(mysql_async_connect *ctx) {
  DBUG_TRACE;
  MYSQL *mysql = ctx->mysql;
  if (ctx->non_blocking) {
    mysql_state_machine_status status = run_plugin_auth_nonblocking(
        ctx->mysql, ctx->scramble_data, ctx->scramble_data_len,
        ctx->scramble_plugin, ctx->db);
    if (status != STATE_MACHINE_DONE) {
      return status;
    }
  } else {
    if (run_plugin_auth(mysql, ctx->scramble_buffer, ctx->scramble_data_len,
                        ctx->scramble_plugin, ctx->db)) {
      return STATE_MACHINE_FAILED;
    }
  }

  if (ctx->scramble_buffer_allocated) {
    ctx->scramble_buffer_allocated = false;
    my_free(ctx->scramble_buffer);
    ctx->scramble_buffer = nullptr;
  }

  ctx->state_function = csm_prep_select_database;
  return STATE_MACHINE_CONTINUE;
}


// Source: client.cc
// Lines 6451-6476
