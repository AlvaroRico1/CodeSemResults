MYSQL_RES *STDCALL mysql_list_processes(MYSQL *mysql) {
  uint field_count;
  uchar *pos;
  DBUG_TRACE;

  if (simple_command(mysql, COM_PROCESS_INFO, nullptr, 0, 0)) return nullptr;
  free_old_query(mysql);
  pos = (uchar *)mysql->net.read_pos;
  field_count = (uint)net_field_length(&pos);
  if (!(mysql->fields =
            cli_read_metadata(mysql, field_count, protocol_41(mysql) ? 7 : 5)))
    return nullptr;
  mysql->status = MYSQL_STATUS_GET_RESULT;
  mysql->field_count = field_count;
  return mysql_store_result(mysql);
}


// Source: libmysql.cc
// Lines 795-810
