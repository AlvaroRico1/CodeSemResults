// Source: curl/lib/multi.c
// Lines 1627-1630
static CURLcode protocol_doing(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  if(conn && conn->handler->doing) {
    *done = FALSE;
    result = conn->handler->doing(data, done);
  }
  else
    *done = TRUE;

  return result;
}
