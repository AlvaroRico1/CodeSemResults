void mysql_trace_trace(MYSQL *m, enum trace_event ev,
                       struct st_trace_event_args args) {
  struct st_mysql_trace_info *trace_info = TRACE_DATA(m);
  struct st_mysql_client_plugin_TRACE *plugin =
      trace_info ? trace_info->plugin : nullptr;
  int quit_tracing = 0;

  /*
    If trace_info is NULL then this connection is not traced and this
    function should not be called - this is handled inside MYSQL_TRACE()
    macro.
  */
  assert(trace_info);

  /* Call plugin's trace_event() method if defined */

  if (plugin->trace_event) {
    /*
      Temporarily disable tracing while executing plugin's method
      by setting trace data pointer to NULL. Also, set reconnect
      flag to 0 in case plugin executes any queries.
    */
    bool saved_reconnect_flag = m->reconnect;

    TRACE_DATA(m) = nullptr;
    m->reconnect = false;
    quit_tracing = plugin->trace_event(plugin, GET_DATA(trace_info), m,
                                       GET_STAGE(trace_info), ev, args);
    m->reconnect = saved_reconnect_flag;
    TRACE_DATA(m) = trace_info;
  }


// Source: mysql_trace.cc
// Lines 138-168
