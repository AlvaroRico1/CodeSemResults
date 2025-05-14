// Source: curl/lib/multi.c
// Lines 1512-1515
static CURLcode multi_do(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  DEBUGASSERT(conn);
  DEBUGASSERT(conn->handler);

  if(conn->handler->do_it)
    /* generic protocol-specific function pointer set in curl_connect() */
    result = conn->handler->do_it(data, done);

  return result;
}
