// Source: curl/lib/http.c
// Lines 1577-1580
static CURLcode https_connecting(struct Curl_easy *data, bool *done)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  DEBUGASSERT((data) && (data->conn->handler->flags & PROTOPT_SSL));

#ifdef ENABLE_QUIC
  if(conn->transport == TRNSPRT_QUIC) {
    *done = TRUE;
    return CURLE_OK;
  }
#endif

  /* perform SSL initialization for this socket */
  result = Curl_ssl_connect_nonblocking(data, conn, FALSE, FIRSTSOCKET, done);
  if(result)
    connclose(conn, "Failed HTTPS connection");

  return result;
}
