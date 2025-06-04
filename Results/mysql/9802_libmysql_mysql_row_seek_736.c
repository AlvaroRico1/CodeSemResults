MYSQL_ROW_OFFSET STDCALL mysql_row_seek(MYSQL_RES *result,
                                        MYSQL_ROW_OFFSET row) {
  MYSQL_ROW_OFFSET return_value = result->data_cursor;
  result->current_row = nullptr;
  result->data_cursor = row;
  return return_value;
}


// Source: libmysql.cc
// Lines 691-697
