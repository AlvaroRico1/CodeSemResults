void STDCALL mysql_stmt_data_seek(MYSQL_STMT *stmt, uint64_t row) {
  MYSQL_ROWS *tmp = stmt->result.data;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("row id to seek: %ld", (long)row));

  for (; tmp && row; --row, tmp = tmp->next)
    ;
  stmt->data_cursor = tmp;
  if (!row && tmp) {
    /*  Rewind the counter */
    stmt->read_row_func = stmt_read_row_buffered;
    stmt->state = MYSQL_STMT_EXECUTE_DONE;
  }
}


// Source: libmysql.cc
// Lines 4038-4051
