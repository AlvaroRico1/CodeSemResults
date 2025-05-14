// Source: curl/lib/vtls/vtls.c
// Lines 394-398
bool Curl_ssl_getsessionid(struct Curl_easy *data,
                           struct connectdata *conn,
                           const bool isProxy,
                           void **ssl_sessionid,
                           size_t *idsize, /* set 0 if unknown */
                           int sockindex)
{
  struct Curl_ssl_session *check;
  size_t i;
  long *general_age;
  bool no_match = TRUE;

#ifndef CURL_DISABLE_PROXY
  struct ssl_primary_config * const ssl_config = isProxy ?
    &conn->proxy_ssl_config :
    &conn->ssl_config;
  const char * const name = isProxy ?
    conn->http_proxy.host.name : conn->host.name;
  int port = isProxy ? (int)conn->port : conn->remote_port;
#else
  /* no proxy support */
  struct ssl_primary_config * const ssl_config = &conn->ssl_config;
  const char * const name = conn->host.name;
  int port = conn->remote_port;
#endif
  (void)sockindex;
  *ssl_sessionid = NULL;

#ifdef CURL_DISABLE_PROXY
  if(isProxy)
    return TRUE;
#endif

  DEBUGASSERT(SSL_SET_OPTION(primary.sessionid));

  if(!SSL_SET_OPTION(primary.sessionid) || !data->state.session)
    /* session ID re-use is disabled or the session cache has not been
       setup */
    return TRUE;

  /* Lock if shared */
  if(SSLSESSION_SHARED(data))
    general_age = &data->share->sessionage;
  else
    general_age = &data->state.sessionage;

  for(i = 0; i < data->set.general_ssl.max_ssl_sessions; i++) {
    check = &data->state.session[i];
    if(!check->sessionid)
      /* not session ID means blank entry */
      continue;
    if(strcasecompare(name, check->name) &&
       ((!conn->bits.conn_to_host && !check->conn_to_host) ||
        (conn->bits.conn_to_host && check->conn_to_host &&
         strcasecompare(conn->conn_to_host.name, check->conn_to_host))) &&
       ((!conn->bits.conn_to_port && check->conn_to_port == -1) ||
        (conn->bits.conn_to_port && check->conn_to_port != -1 &&
         conn->conn_to_port == check->conn_to_port)) &&
       (port == check->remote_port) &&
       strcasecompare(conn->handler->scheme, check->scheme) &&
       Curl_ssl_config_matches(ssl_config, &check->ssl_config)) {
      /* yes, we have a session ID! */
      (*general_age)++;          /* increase general age */
      check->age = *general_age; /* set this as used in this age */
      *ssl_sessionid = check->sessionid;
      if(idsize)
        *idsize = check->idsize;
      no_match = FALSE;
      break;
    }
  }

  DEBUGF(infof(data, "%s Session ID in cache for %s %s://%s:%d",
               no_match? "Didn't find": "Found",
               isProxy ? "proxy" : "host",
               conn->handler->scheme, name, port));
  return no_match;
}
