CURLcode Curl_proxy_connect(struct Curl_easy *data, int sockindex)
{
  struct connectdata *conn = data->conn;
  if(conn->http_proxy.proxytype == CURLPROXY_HTTPS) {
    const CURLcode result = https_proxy_connect(data, sockindex);
    if(result)
      return result;
    if(!conn->bits.proxy_ssl_connected[sockindex])
      return result; /* wait for HTTPS proxy SSL initialization to complete */
  }

  if(conn->bits.tunnel_proxy && conn->bits.httpproxy) {
#ifndef CURL_DISABLE_PROXY
    /* for [protocol] tunneled through HTTP proxy */
    const char *hostname;
    int remote_port;
    CURLcode result;

    /* We want "seamless" operations through HTTP proxy tunnel */

    /* for the secondary socket (FTP), use the "connect to host"
     * but ignore the "connect to port" (use the secondary port)
     */

    if(conn->bits.conn_to_host)
      hostname = conn->conn_to_host.name;
    else if(sockindex == SECONDARYSOCKET)
      hostname = conn->secondaryhostname;
    else
      hostname = conn->host.name;

    if(sockindex == SECONDARYSOCKET)
      remote_port = conn->secondary_port;
    else if(conn->bits.conn_to_port)
      remote_port = conn->conn_to_port;
    else
      remote_port = conn->remote_port;

    result = Curl_proxyCONNECT(data, sockindex, hostname, remote_port);
    if(CURLE_OK != result)
      return result;
    Curl_safefree(data->state.aptr.proxyuserpwd);
#else
    return CURLE_NOT_BUILT_IN;
#endif
  }
  /* no HTTP tunnel proxy, just return */
  return CURLE_OK;
}


// Source: http_proxy.c
// Lines 79-127
