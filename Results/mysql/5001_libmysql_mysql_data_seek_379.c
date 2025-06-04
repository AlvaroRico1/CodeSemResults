void STDCALL mysql_data_seek(MYSQL_RES *result, uint64_t row) {
  MYSQL_ROWS *tmp = nullptr;
  DBUG_PRINT("info", ("mysql_data_seek(%ld)", (long)row));
  if (result->data)
    for (tmp = result->data->data; row-- && tmp; tmp = tmp->next)
      ;
  result->current_row = nullptr;
  result->data_cursor = tmp;
}


// Source: libmysql.cc
// Lines 675-683
