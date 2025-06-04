void STDCALL mysql_free_result(MYSQL_RES *result) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("mysql_res: %p", result));
  if (result) {
    MYSQL *mysql = result->handle;
    if (mysql) {
      if (mysql->unbuffered_fetch_owner == &result->unbuffered_fetch_cancelled)
        mysql->unbuffered_fetch_owner = nullptr;
      if (mysql->status == MYSQL_STATUS_USE_RESULT) {
        (*mysql->methods->flush_use_result)(mysql, false);
        mysql->status = MYSQL_STATUS_READY;
        if (mysql->unbuffered_fetch_owner)
          *mysql->unbuffered_fetch_owner = true;
      }
    }
    free_rows(result->data);
    if (result->field_alloc) {
      free_root(result->field_alloc, MYF(0));
      my_free(result->field_alloc);
    }
    my_free(result->row);
    my_free(result);
  }


// Source: client.cc
// Lines 1889-1911
