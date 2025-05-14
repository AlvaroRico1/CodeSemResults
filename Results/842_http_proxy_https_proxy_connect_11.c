// Source: curl/lib/http_proxy.c
// Lines 55-58
static CURLcode https_proxy_connect(struct Curl_easy *data, int sockindex)
{
#ifdef USE_SSL
  struct connectdata *conn = data->conn;
  CURLcode result = CURLE_OK;
  DEBUGASSERT(conn->http_proxy.proxytype == CURLPROXY_HTTPS);
  if(!conn->bits.proxy_ssl_connected[sockindex]) {
    /* perform SSL initialization for this socket */
    result =
      Curl_ssl_connect_nonblocking(data, conn, TRUE, sockindex,
                                   &conn->bits.proxy_ssl_connected[sockindex]);
    if(result)
      /* a failed connection is marked for closure to prevent (bad) re-use or
         similar */
      connclose(conn, "TLS handshake failed");
  }
  return result;
#else
  (void) data;
  (void) sockindex;
  return CURLE_NOT_BUILT_IN;
#endif
}
