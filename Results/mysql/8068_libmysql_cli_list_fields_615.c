MYSQL_FIELD *cli_list_fields(MYSQL *mysql) {
  MYSQL_DATA *query;
  MYSQL_FIELD *result;

  MYSQL_TRACE_STAGE(mysql, WAIT_FOR_FIELD_DEF);
  query =
      cli_read_rows(mysql, (MYSQL_FIELD *)nullptr, protocol_41(mysql) ? 8 : 6);
  MYSQL_TRACE_STAGE(mysql, READY_FOR_COMMAND);

  if (!query) return nullptr;

  mysql->field_count = (uint)query->rows;
  result = unpack_fields(mysql, query->data, mysql->field_alloc,
                         mysql->field_count, true, mysql->server_capabilities);
  free_rows(query);
  return result;
}


// Source: libmysql.cc
// Lines 733-749
