static CURLcode resolve_server(struct Curl_easy *data,
                               struct connectdata *conn,
                               bool *async)
{
  CURLcode result = CURLE_OK;
  timediff_t timeout_ms = Curl_timeleft(data, NULL, TRUE);

  DEBUGASSERT(conn);
  DEBUGASSERT(data);
  /*************************************************************
   * Resolve the name of the server or proxy
   *************************************************************/
  if(conn->bits.reuse)
    /* We're reusing the connection - no need to resolve anything, and
       idnconvert_hostname() was called already in create_conn() for the re-use
       case. */
    *async = FALSE;

  else {
    /* this is a fresh connect */
    int rc;
    struct Curl_dns_entry *hostaddr = NULL;

#ifdef USE_UNIX_SOCKETS
    if(conn->unix_domain_socket) {
      /* Unix domain sockets are local. The host gets ignored, just use the
       * specified domain socket address. Do not cache "DNS entries". There is
       * no DNS involved and we already have the filesystem path available */
      const char *path = conn->unix_domain_socket;

      hostaddr = calloc(1, sizeof(struct Curl_dns_entry));
      if(!hostaddr)
        result = CURLE_OUT_OF_MEMORY;
      else {
        bool longpath = FALSE;
        hostaddr->addr = Curl_unix2addr(path, &longpath,
                                        conn->bits.abstract_unix_socket);
        if(hostaddr->addr)
          hostaddr->inuse++;
        else {
          /* Long paths are not supported for now */
          if(longpath) {
            failf(data, "Unix socket path too long: '%s'", path);
            result = CURLE_COULDNT_RESOLVE_HOST;
          }
          else
            result = CURLE_OUT_OF_MEMORY;
          free(hostaddr);
          hostaddr = NULL;
        }
      }
    }
    else
#endif

    if(!conn->bits.proxy) {
      struct hostname *connhost;
      if(conn->bits.conn_to_host)
        connhost = &conn->conn_to_host;
      else
        connhost = &conn->host;

      /* If not connecting via a proxy, extract the port from the URL, if it is
       * there, thus overriding any defaults that might have been set above. */
      if(conn->bits.conn_to_port)
        conn->port = conn->conn_to_port;
      else
        conn->port = conn->remote_port;

      /* Resolve target host right on */
      conn->hostname_resolve = strdup(connhost->name);
      if(!conn->hostname_resolve)
        return CURLE_OUT_OF_MEMORY;
      rc = Curl_resolv_timeout(data, conn->hostname_resolve, (int)conn->port,
                               &hostaddr, timeout_ms);
      if(rc == CURLRESOLV_PENDING)
        *async = TRUE;

      else if(rc == CURLRESOLV_TIMEDOUT) {
        failf(data, "Failed to resolve host '%s' with timeout after %ld ms",
              connhost->dispname,
              Curl_timediff(Curl_now(), data->progress.t_startsingle));
        result = CURLE_OPERATION_TIMEDOUT;
      }
      else if(!hostaddr) {
        failf(data, "Could not resolve host: %s", connhost->dispname);
        result = CURLE_COULDNT_RESOLVE_HOST;
        /* don't return yet, we need to clean up the timeout first */
      }
    }
#ifndef CURL_DISABLE_PROXY
    else {
      /* This is a proxy that hasn't been resolved yet. */

      struct hostname * const host = conn->bits.socksproxy ?
        &conn->socks_proxy.host : &conn->http_proxy.host;

      /* resolve proxy */
      conn->hostname_resolve = strdup(host->name);
      if(!conn->hostname_resolve)
        return CURLE_OUT_OF_MEMORY;
      rc = Curl_resolv_timeout(data, conn->hostname_resolve, (int)conn->port,
                               &hostaddr, timeout_ms);

      if(rc == CURLRESOLV_PENDING)
        *async = TRUE;

      else if(rc == CURLRESOLV_TIMEDOUT)
        result = CURLE_OPERATION_TIMEDOUT;

      else if(!hostaddr) {
        failf(data, "Couldn't resolve proxy '%s'", host->dispname);
        result = CURLE_COULDNT_RESOLVE_PROXY;
        /* don't return yet, we need to clean up the timeout first */
      }
    }
#endif
    DEBUGASSERT(conn->dns_entry == NULL);
    conn->dns_entry = hostaddr;
  }


// Source: url.c
// Lines 3307-3426
