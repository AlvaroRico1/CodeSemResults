static CURLcode multi_do_more(struct Curl_easy *data, int *complete)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  *complete = 0;

  if(conn->handler->do_more)
    result = conn->handler->do_more(data, complete);

  return result;
}


// Source: multi.c
// Lines 1536-1547
