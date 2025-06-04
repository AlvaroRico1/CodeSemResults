MYSQL_ROW STDCALL mysql_fetch_row(MYSQL_RES *res) {
  DBUG_TRACE;
  if (!res->data) { /* Unbufferred fetch */
    if (!res->eof) {
      MYSQL *mysql = res->handle;
      if (mysql->status != MYSQL_STATUS_USE_RESULT) {
        set_mysql_error(mysql,
                        res->unbuffered_fetch_cancelled
                            ? CR_FETCH_CANCELED
                            : CR_COMMANDS_OUT_OF_SYNC,
                        unknown_sqlstate);
      } else if (!(read_one_row(mysql, res->field_count, res->row,
                                res->lengths))) {
        res->row_count++;
        return res->current_row = res->row;
      }
      DBUG_PRINT("info", ("end of data"));
      res->eof = true;
      mysql->status = MYSQL_STATUS_READY;
      /*
        Reset only if owner points to us: there is a chance that somebody
        started new query after mysql_stmt_close():
      */
      if (mysql->unbuffered_fetch_owner == &res->unbuffered_fetch_cancelled)
        mysql->unbuffered_fetch_owner = nullptr;
      /* Don't clear handle in mysql_free_result */
      res->handle = nullptr;
    }


// Source: client.cc
// Lines 7714-7741
