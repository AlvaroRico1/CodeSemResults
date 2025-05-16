static CURLcode protocol_connecting(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  if(conn && conn->handler->connecting) {
    *done = FALSE;
    result = conn->handler->connecting(data, done);
  }
  else
    *done = TRUE;

  return result;
}


// Source: multi.c
// Lines 1607-1620
