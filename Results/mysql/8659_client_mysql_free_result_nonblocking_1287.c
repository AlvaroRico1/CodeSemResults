net_async_status STDCALL mysql_free_result_nonblocking(MYSQL_RES *result) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("mysql_res: %p", result));
  if (!result) return NET_ASYNC_COMPLETE;

  MYSQL *mysql = result->handle;
  if (mysql) {
    if (mysql->unbuffered_fetch_owner == &result->unbuffered_fetch_cancelled)
      mysql->unbuffered_fetch_owner = nullptr;
    if (mysql->status == MYSQL_STATUS_USE_RESULT) {
      if (mysql->methods->flush_use_result_nonblocking(mysql, false) ==
          NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      mysql->status = MYSQL_STATUS_READY;
      if (mysql->unbuffered_fetch_owner) *mysql->unbuffered_fetch_owner = true;
    }
  }
  free_rows(result->data);
  if (result->field_alloc) {
    free_root(result->field_alloc, MYF(0));
    my_free(result->field_alloc);
  }
  my_free(result->row);
  my_free(result);

  return NET_ASYNC_COMPLETE;
}


// Source: client.cc
// Lines 1860-1887
