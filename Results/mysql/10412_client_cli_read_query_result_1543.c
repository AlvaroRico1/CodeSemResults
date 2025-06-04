static bool cli_read_query_result(MYSQL *mysql) {
  uchar *pos;
  ulong field_count;
  ulong length;
  DBUG_TRACE;

  if ((length = cli_safe_read(mysql, nullptr)) == packet_error) return true;
  free_old_query(mysql); /* Free old result */
#ifndef MYSQL_SERVER     /* Avoid warn of unused labels*/
get_info:
#endif
  pos = (uchar *)mysql->net.read_pos;
  if ((field_count = net_field_length(&pos)) == 0) {
    read_ok_ex(mysql, length);
#if defined(CLIENT_PROTOCOL_TRACING)
    if (mysql->server_status & SERVER_MORE_RESULTS_EXISTS)
      MYSQL_TRACE_STAGE(mysql, WAIT_FOR_RESULT);
    else
      MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);
#endif
    return false;
  }
#ifndef MYSQL_SERVER
  if (field_count == NULL_LENGTH) /* LOAD DATA LOCAL INFILE */
  {
    int error;

    MYSQL_TRACE_STAGE(mysql, FILE_REQUEST);

    error = handle_local_infile(mysql, (char *)pos);

    MYSQL_TRACE_STAGE(mysql, WAIT_FOR_RESULT);

    if ((length = cli_safe_read(mysql, nullptr)) == packet_error || error)
      return true;
    goto get_info; /* Get info packet */
  }
#endif
  if (!(mysql->server_status & SERVER_STATUS_AUTOCOMMIT))
    mysql->server_status |= SERVER_STATUS_IN_TRANS;

  if (read_com_query_metadata(mysql, pos, field_count)) return true;

  mysql->status = MYSQL_STATUS_GET_RESULT;
  mysql->field_count = (uint)field_count;

  MYSQL_TRACE_STAGE(mysql, WAIT_FOR_ROW);

  DBUG_PRINT("exit", ("ok"));
  return false;
}


// Source: client.cc
// Lines 7045-7095
