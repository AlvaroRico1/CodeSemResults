static MYSQL_RES *cli_use_result(MYSQL *mysql) {
  MYSQL_RES *result;
  DBUG_TRACE;

  /*
    Some queries (e.g. "CALL") may return an empty resultset.
    mysql->field_count is 0 in such cases.
  */
  if (!mysql->field_count) return nullptr;
  if (mysql->status != MYSQL_STATUS_GET_RESULT) {
    set_mysql_error(mysql, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate);
    return nullptr;
  }
  if (!(result = (MYSQL_RES *)my_malloc(
            key_memory_MYSQL_RES,
            sizeof(*result) + sizeof(ulong) * mysql->field_count,
            MYF(MY_WME | MY_ZEROFILL))))
    return nullptr;
  result->lengths = (ulong *)(result + 1);
  result->methods = mysql->methods;
  if (!(result->row = (MYSQL_ROW)my_malloc(
            key_memory_MYSQL_ROW,
            sizeof(result->row[0]) * (mysql->field_count + 1),
            MYF(MY_WME)))) { /* Ptrs: to one row */
    my_free(result);
    return nullptr;
  }
  if (!(result->field_alloc = (MEM_ROOT *)my_malloc(
            key_memory_MYSQL, sizeof(MEM_ROOT), MYF(MY_WME | MY_ZEROFILL)))) {
    my_free(result->row);
    my_free(result);
    return nullptr;
  }
  result->fields = mysql->fields;
  *result->field_alloc = std::move(*mysql->field_alloc);
  result->field_count = mysql->field_count;
  result->metadata = mysql->resultset_metadata;
  result->current_field = 0;
  result->handle = mysql;
  result->current_row = nullptr;
  mysql->fields = nullptr; /* fields is now in result */
  mysql->status = MYSQL_STATUS_USE_RESULT;
  mysql->unbuffered_fetch_owner = &result->unbuffered_fetch_cancelled;
  return result; /* Data is read to be fetched */
}


// Source: client.cc
// Lines 7664-7708
