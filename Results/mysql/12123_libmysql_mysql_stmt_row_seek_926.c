MYSQL_ROW_OFFSET STDCALL mysql_stmt_row_seek(MYSQL_STMT *stmt,
                                             MYSQL_ROW_OFFSET row) {
  MYSQL_ROW_OFFSET offset = stmt->data_cursor;
  DBUG_TRACE;

  stmt->data_cursor = row;
  return offset;
}


// Source: libmysql.cc
// Lines 4015-4022
