static net_async_status mysql_send_query_nonblocking_inner(MYSQL *mysql,
                                                           const char *query,
                                                           ulong length) {
  DBUG_TRACE;
  STATE_INFO *info;

  if ((info = STATE_DATA(mysql)))
    free_state_change_info(static_cast<MYSQL_EXTENSION *>(mysql->extension));

  bool ret;
  MYSQL_ASYNC *async_context = ASYNC_DATA(mysql);

  if ((*mysql->methods->advanced_command_nonblocking)(
          mysql, COM_QUERY, async_context->async_qp_data,
          async_context->async_qp_data_length,
          pointer_cast<const uchar *>(query), length, 1, NULL,
          &ret) == NET_ASYNC_NOT_READY) {
    return NET_ASYNC_NOT_READY;
  }
  if (ret)
    return NET_ASYNC_ERROR;
  else
    return NET_ASYNC_COMPLETE;
}


// Source: client.cc
// Lines 7319-7342
