CURLproxycode Curl_SOCKS4(const char *proxy_user,
                          const char *hostname,
                          int remote_port,
                          int sockindex,
                          struct Curl_easy *data,
                          bool *done)
{
  struct connectdata *conn = data->conn;
  const bool protocol4a =
    (conn->socks_proxy.proxytype == CURLPROXY_SOCKS4A) ? TRUE : FALSE;
  unsigned char *socksreq = (unsigned char *)data->state.buffer;
  CURLcode result;
  curl_socket_t sockfd = conn->sock[sockindex];
  struct connstate *sx = &conn->cnnct;
  struct Curl_dns_entry *dns = NULL;
  ssize_t actualread;
  ssize_t written;

  /* make sure that the buffer is at least 600 bytes */
  DEBUGASSERT(READBUFFER_MIN >= 600);

  if(!SOCKS_STATE(sx->state) && !*done)
    sxstate(data, CONNECT_SOCKS_INIT);

  switch(sx->state) {
  case CONNECT_SOCKS_INIT:
    /* SOCKS4 can only do IPv4, insist! */
    conn->ip_version = CURL_IPRESOLVE_V4;
    if(conn->bits.httpproxy)
      infof(data, "SOCKS4%s: connecting to HTTP proxy %s port %d",
            protocol4a ? "a" : "", hostname, remote_port);

    infof(data, "SOCKS4 communication to %s:%d", hostname, remote_port);

    /*
     * Compose socks4 request
     *
     * Request format
     *
     *     +----+----+----+----+----+----+----+----+----+----+....+----+
     *     | VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
     *     +----+----+----+----+----+----+----+----+----+----+....+----+
     * # of bytes:  1    1      2              4           variable       1
     */

    socksreq[0] = 4; /* version (SOCKS4) */
    socksreq[1] = 1; /* connect */
    socksreq[2] = (unsigned char)((remote_port >> 8) & 0xff); /* PORT MSB */
    socksreq[3] = (unsigned char)(remote_port & 0xff);        /* PORT LSB */

    /* DNS resolve only for SOCKS4, not SOCKS4a */
    if(!protocol4a) {
      enum resolve_t rc =
        Curl_resolv(data, hostname, remote_port, FALSE, &dns);

      if(rc == CURLRESOLV_ERROR)
        return CURLPX_RESOLVE_HOST;
      else if(rc == CURLRESOLV_PENDING) {
        sxstate(data, CONNECT_RESOLVING);
        infof(data, "SOCKS4 non-blocking resolve of %s", hostname);
        return CURLPX_OK;
      }
      sxstate(data, CONNECT_RESOLVED);
      goto CONNECT_RESOLVED;
    }

    /* socks4a doesn't resolve anything locally */
    sxstate(data, CONNECT_REQ_INIT);
    goto CONNECT_REQ_INIT;

  case CONNECT_RESOLVING:
    /* check if we have the name resolved by now */
    dns = Curl_fetch_addr(data, hostname, (int)conn->port);

    if(dns) {
#ifdef CURLRES_ASYNCH
      data->state.async.dns = dns;
      data->state.async.done = TRUE;
#endif
      infof(data, "Hostname '%s' was found", hostname);
      sxstate(data, CONNECT_RESOLVED);
    }
    else {
      result = Curl_resolv_check(data, &dns);
      if(!dns) {
        if(result)
          return CURLPX_RESOLVE_HOST;
        return CURLPX_OK;
      }
    }
    /* FALLTHROUGH */
  CONNECT_RESOLVED:
  case CONNECT_RESOLVED: {
    struct Curl_addrinfo *hp = NULL;
    /*
     * We cannot use 'hostent' as a struct that Curl_resolv() returns.  It
     * returns a Curl_addrinfo pointer that may not always look the same.
     */
    if(dns) {
      hp = dns->addr;

      /* scan for the first IPv4 address */
      while(hp && (hp->ai_family != AF_INET))
        hp = hp->ai_next;

      if(hp) {
        struct sockaddr_in *saddr_in;
        char buf[64];
        Curl_printable_address(hp, buf, sizeof(buf));

        saddr_in = (struct sockaddr_in *)(void *)hp->ai_addr;
        socksreq[4] = ((unsigned char *)&saddr_in->sin_addr.s_addr)[0];
        socksreq[5] = ((unsigned char *)&saddr_in->sin_addr.s_addr)[1];
        socksreq[6] = ((unsigned char *)&saddr_in->sin_addr.s_addr)[2];
        socksreq[7] = ((unsigned char *)&saddr_in->sin_addr.s_addr)[3];

        infof(data, "SOCKS4 connect to IPv4 %s (locally resolved)", buf);

        Curl_resolv_unlock(data, dns); /* not used anymore from now on */
      }


// Source: socks.c
// Lines 188-307
