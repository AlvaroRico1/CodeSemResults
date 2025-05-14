// Source: curl/lib/connect.c
// Lines 770-776
static CURLcode connect_SOCKS(struct Curl_easy *data, int sockindex,
                              bool *done)
{
  CURLcode result = CURLE_OK;
#ifndef CURL_DISABLE_PROXY
  CURLproxycode pxresult = CURLPX_OK;
  struct connectdata *conn = data->conn;
  if(conn->bits.socksproxy) {
    /* for the secondary socket (FTP), use the "connect to host"
     * but ignore the "connect to port" (use the secondary port)
     */
    const char * const host =
      conn->bits.httpproxy ?
      conn->http_proxy.host.name :
      conn->bits.conn_to_host ?
      conn->conn_to_host.name :
      sockindex == SECONDARYSOCKET ?
      conn->secondaryhostname : conn->host.name;
    const int port =
      conn->bits.httpproxy ? (int)conn->http_proxy.port :
      sockindex == SECONDARYSOCKET ? conn->secondary_port :
      conn->bits.conn_to_port ? conn->conn_to_port :
      conn->remote_port;
    switch(conn->socks_proxy.proxytype) {
    case CURLPROXY_SOCKS5:
    case CURLPROXY_SOCKS5_HOSTNAME:
      pxresult = Curl_SOCKS5(conn->socks_proxy.user, conn->socks_proxy.passwd,
                             host, port, sockindex, data, done);
      break;

    case CURLPROXY_SOCKS4:
    case CURLPROXY_SOCKS4A:
      pxresult = Curl_SOCKS4(conn->socks_proxy.user, host, port, sockindex,
                             data, done);
      break;

    default:
      failf(data, "unknown proxytype option given");
      result = CURLE_COULDNT_CONNECT;
    } /* switch proxytype */
    if(pxresult) {
      result = CURLE_PROXY;
      data->info.pxcode = pxresult;
    }
  }
  else
#else
    (void)data;
    (void)sockindex;
#endif /* CURL_DISABLE_PROXY */
    *done = TRUE; /* no SOCKS proxy, so consider us connected */

  return result;
}
