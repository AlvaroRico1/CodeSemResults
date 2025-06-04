int STDCALL mysql_stmt_fetch(MYSQL_STMT *stmt) {
  int rc;
  uchar *row;
  DBUG_TRACE;

  if ((rc = (*stmt->read_row_func)(stmt, &row)) ||
      ((rc = stmt_fetch_row(stmt, row)) && rc != MYSQL_DATA_TRUNCATED)) {
    stmt->state = MYSQL_STMT_PREPARE_DONE; /* XXX: this is buggy */
    stmt->read_row_func = (rc == MYSQL_NO_DATA) ? stmt_read_row_no_data
                                                : stmt_read_row_no_result_set;
  } else {
    /* This is to know in mysql_stmt_fetch_column that data was fetched */
    stmt->state = MYSQL_STMT_FETCH_DONE;
  }
  return rc;
}


// Source: libmysql.cc
// Lines 3735-3750
