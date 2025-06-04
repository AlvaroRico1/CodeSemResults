void mysql_trace_start(MYSQL *m) {
  struct st_mysql_trace_info *trace_info;

  trace_info = (st_mysql_trace_info *)my_malloc(
      PSI_NOT_INSTRUMENTED, sizeof(struct st_mysql_trace_info),
      MYF(MY_ZEROFILL));
  if (!trace_info) {
    /*
      Note: in this case trace_data of the connection will
      remain NULL and thus tracing will be disabled.
    */
    return;
  }

  /*
    This function should be called only when a trace plugin
    is loaded and thus trace_plugin pointer is not NULL. This
    is handled in MYSQL_TRACE_STAGE() macro (mysql_trace.h).
  */
  assert(trace_plugin);

  trace_info->plugin = trace_plugin;
  trace_info->stage = PROTOCOL_STAGE_CONNECTING;

  /*
    Call plugin's tracing_start() method, if defined.
  */

  if (trace_info->plugin->tracing_start) {
    trace_info->trace_plugin_data = trace_info->plugin->tracing_start(
        trace_info->plugin, m, PROTOCOL_STAGE_CONNECTING);
  } else {
    trace_info->trace_plugin_data = nullptr;
  }

  /* Store trace_info in the connection handle. */

  TRACE_DATA(m) = trace_info;
}


// Source: mysql_trace.cc
// Lines 85-123
