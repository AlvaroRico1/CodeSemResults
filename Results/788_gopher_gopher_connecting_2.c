// Source: curl/lib/gopher.c
// Lines 115-117
static CURLcode gopher_connecting(struct Curl_easy *data, bool *done)
{
  struct connectdata *conn = data->conn;
  CURLcode result = Curl_ssl_connect(data, conn, FIRSTSOCKET);
  if(result)
    connclose(conn, "Failed TLS connection");
  *done = TRUE;
  return result;
}
