CURLcode Curl_http_connect(struct Curl_easy *data, bool *done)
{
  CURLcode result;
  struct connectdata *conn = data->conn;

  /* We default to persistent connections. We set this already in this connect
     function to make the re-use checks properly be able to check this bit. */
  connkeep(conn, "HTTP default");

#ifndef CURL_DISABLE_PROXY
  /* the CONNECT procedure might not have been completed */
  result = Curl_proxy_connect(data, FIRSTSOCKET);
  if(result)
    return result;

  if(conn->bits.proxy_connect_closed)
    /* this is not an error, just part of the connection negotiation */
    return CURLE_OK;

  if(CONNECT_FIRSTSOCKET_PROXY_SSL())
    return CURLE_OK; /* wait for HTTPS proxy SSL initialization to complete */

  if(Curl_connect_ongoing(conn))
    /* nothing else to do except wait right now - we're not done here. */
    return CURLE_OK;

  if(data->set.haproxyprotocol) {
    /* add HAProxy PROXY protocol header */
    result = add_haproxy_protocol_header(data);
    if(result)
      return result;
  }
#endif

  if(conn->given->protocol & CURLPROTO_HTTPS) {
    /* perform SSL initialization */
    result = https_connecting(data, done);
    if(result)
      return result;
  }
  else
    *done = TRUE;

  return CURLE_OK;
}


// Source: http.c
// Lines 1481-1525
