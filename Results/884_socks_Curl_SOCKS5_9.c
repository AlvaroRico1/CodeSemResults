// Source: curl/lib/socks.c
// Lines 797-817
CURLproxycode Curl_SOCKS5(const char *proxy_user,
                          const char *proxy_password,
                          const char *hostname,
                          int remote_port,
                          int sockindex,
                          struct Curl_easy *data,
                          bool *done)
{
  /*
    According to the RFC1928, section "6.  Replies". This is what a SOCK5
    replies:

        +----+-----+-------+------+----------+----------+
        |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

    Where:

    o  VER    protocol version: X'05'
    o  REP    Reply field:
    o  X'00' succeeded
  */
  struct connectdata *conn = data->conn;
  unsigned char *socksreq = (unsigned char *)data->state.buffer;
  char dest[256] = "unknown";  /* printable hostname:port */
  int idx;
  ssize_t actualread;
  ssize_t written;
  CURLcode result;
  curl_socket_t sockfd = conn->sock[sockindex];
  bool socks5_resolve_local =
    (conn->socks_proxy.proxytype == CURLPROXY_SOCKS5) ? TRUE : FALSE;
  const size_t hostname_len = strlen(hostname);
  ssize_t len = 0;
  const unsigned long auth = data->set.socks5auth;
  bool allow_gssapi = FALSE;
  struct connstate *sx = &conn->cnnct;
  struct Curl_dns_entry *dns = NULL;

  if(!SOCKS_STATE(sx->state) && !*done)
    sxstate(data, CONNECT_SOCKS_INIT);

  switch(sx->state) {
  case CONNECT_SOCKS_INIT:
    if(conn->bits.httpproxy)
      infof(data, "SOCKS5: connecting to HTTP proxy %s port %d",
            hostname, remote_port);

    /* RFC1928 chapter 5 specifies max 255 chars for domain name in packet */
    if(!socks5_resolve_local && hostname_len > 255) {
      infof(data, "SOCKS5: server resolving disabled for hostnames of "
            "length > 255 [actual len=%zu]", hostname_len);
      socks5_resolve_local = TRUE;
    }

    if(auth & ~(CURLAUTH_BASIC | CURLAUTH_GSSAPI))
      infof(data,
            "warning: unsupported value passed to CURLOPT_SOCKS5_AUTH: %lu",
            auth);
    if(!(auth & CURLAUTH_BASIC))
      /* disable username/password auth */
      proxy_user = NULL;
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
    if(auth & CURLAUTH_GSSAPI)
      allow_gssapi = TRUE;
#endif

    idx = 0;
    socksreq[idx++] = 5;   /* version */
    idx++;                 /* number of authentication methods */
    socksreq[idx++] = 0;   /* no authentication */
    if(allow_gssapi)
      socksreq[idx++] = 1; /* GSS-API */
    if(proxy_user)
      socksreq[idx++] = 2; /* username/password */
    /* write the number of authentication methods */
    socksreq[1] = (unsigned char) (idx - 2);

    result = Curl_write_plain(data, sockfd, (char *)socksreq, idx, &written);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "Unable to send initial SOCKS5 request.");
      return CURLPX_SEND_CONNECT;
    }
    if(written != idx) {
      sxstate(data, CONNECT_SOCKS_SEND);
      sx->outstanding = idx - written;
      sx->outp = &socksreq[written];
      return CURLPX_OK;
    }
    sxstate(data, CONNECT_SOCKS_READ);
    goto CONNECT_SOCKS_READ_INIT;
  case CONNECT_SOCKS_SEND:
    result = Curl_write_plain(data, sockfd, (char *)sx->outp,
                              sx->outstanding, &written);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "Unable to send initial SOCKS5 request.");
      return CURLPX_SEND_CONNECT;
    }
    if(written != sx->outstanding) {
      /* not done, remain in state */
      sx->outstanding -= written;
      sx->outp += written;
      return CURLPX_OK;
    }
    /* FALLTHROUGH */
  CONNECT_SOCKS_READ_INIT:
  case CONNECT_SOCKS_READ_INIT:
    sx->outstanding = 2; /* expect two bytes */
    sx->outp = socksreq; /* store it here */
    /* FALLTHROUGH */
  case CONNECT_SOCKS_READ:
    result = Curl_read_plain(sockfd, (char *)sx->outp,
                             sx->outstanding, &actualread);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "Unable to receive initial SOCKS5 response.");
      return CURLPX_RECV_CONNECT;
    }
    else if(!result && !actualread) {
      /* connection closed */
      failf(data, "Connection to proxy closed");
      return CURLPX_CLOSED;
    }
    else if(actualread != sx->outstanding) {
      /* remain in reading state */
      sx->outstanding -= actualread;
      sx->outp += actualread;
      return CURLPX_OK;
    }
    else if(socksreq[0] != 5) {
      failf(data, "Received invalid version in initial SOCKS5 response.");
      return CURLPX_BAD_VERSION;
    }
    else if(socksreq[1] == 0) {
      /* DONE! No authentication needed. Send request. */
      sxstate(data, CONNECT_REQ_INIT);
      goto CONNECT_REQ_INIT;
    }
    else if(socksreq[1] == 2) {
      /* regular name + password authentication */
      sxstate(data, CONNECT_AUTH_INIT);
      goto CONNECT_AUTH_INIT;
    }
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
    else if(allow_gssapi && (socksreq[1] == 1)) {
      sxstate(data, CONNECT_GSSAPI_INIT);
      result = Curl_SOCKS5_gssapi_negotiate(sockindex, data);
      if(result) {
        failf(data, "Unable to negotiate SOCKS5 GSS-API context.");
        return CURLPX_GSSAPI;
      }
    }
#endif
    else {
      /* error */
      if(!allow_gssapi && (socksreq[1] == 1)) {
        failf(data,
              "SOCKS5 GSSAPI per-message authentication is not supported.");
        return CURLPX_GSSAPI_PERMSG;
      }
      else if(socksreq[1] == 255) {
        failf(data, "No authentication method was acceptable.");
        return CURLPX_NO_AUTH;
      }
    }
    failf(data,
          "Undocumented SOCKS5 mode attempted to be used by server.");
    return CURLPX_UNKNOWN_MODE;
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
  case CONNECT_GSSAPI_INIT:
    /* GSSAPI stuff done non-blocking */
    break;
#endif

  default: /* do nothing! */
    break;

  CONNECT_AUTH_INIT:
  case CONNECT_AUTH_INIT: {
    /* Needs user name and password */
    size_t proxy_user_len, proxy_password_len;
    if(proxy_user && proxy_password) {
      proxy_user_len = strlen(proxy_user);
      proxy_password_len = strlen(proxy_password);
    }
    else {
      proxy_user_len = 0;
      proxy_password_len = 0;
    }

    /*   username/password request looks like
     * +----+------+----------+------+----------+
     * |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
     * +----+------+----------+------+----------+
     * | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
     * +----+------+----------+------+----------+
     */
    len = 0;
    socksreq[len++] = 1;    /* username/pw subnegotiation version */
    socksreq[len++] = (unsigned char) proxy_user_len;
    if(proxy_user && proxy_user_len) {
      /* the length must fit in a single byte */
      if(proxy_user_len >= 255) {
        failf(data, "Excessive user name length for proxy auth");
        return CURLPX_LONG_USER;
      }
      memcpy(socksreq + len, proxy_user, proxy_user_len);
    }
    len += proxy_user_len;
    socksreq[len++] = (unsigned char) proxy_password_len;
    if(proxy_password && proxy_password_len) {
      /* the length must fit in a single byte */
      if(proxy_password_len > 255) {
        failf(data, "Excessive password length for proxy auth");
        return CURLPX_LONG_PASSWD;
      }
      memcpy(socksreq + len, proxy_password, proxy_password_len);
    }
    len += proxy_password_len;
    sxstate(data, CONNECT_AUTH_SEND);
    sx->outstanding = len;
    sx->outp = socksreq;
  }
    /* FALLTHROUGH */
  case CONNECT_AUTH_SEND:
    result = Curl_write_plain(data, sockfd, (char *)sx->outp,
                              sx->outstanding, &written);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "Failed to send SOCKS5 sub-negotiation request.");
      return CURLPX_SEND_AUTH;
    }
    if(sx->outstanding != written) {
      /* remain in state */
      sx->outstanding -= written;
      sx->outp += written;
      return CURLPX_OK;
    }
    sx->outp = socksreq;
    sx->outstanding = 2;
    sxstate(data, CONNECT_AUTH_READ);
    /* FALLTHROUGH */
  case CONNECT_AUTH_READ:
    result = Curl_read_plain(sockfd, (char *)sx->outp,
                             sx->outstanding, &actualread);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "Unable to receive SOCKS5 sub-negotiation response.");
      return CURLPX_RECV_AUTH;
    }
    else if(!result && !actualread) {
      /* connection closed */
      failf(data, "connection to proxy closed");
      return CURLPX_CLOSED;
    }
    else if(actualread != sx->outstanding) {
      /* remain in state */
      sx->outstanding -= actualread;
      sx->outp += actualread;
      return CURLPX_OK;
    }
    /* ignore the first (VER) byte */
    else if(socksreq[1]) { /* status */
      failf(data, "User was rejected by the SOCKS5 server (%d %d).",
            socksreq[0], socksreq[1]);
      return CURLPX_USER_REJECTED;
    }

    /* Everything is good so far, user was authenticated! */
    sxstate(data, CONNECT_REQ_INIT);
    /* FALLTHROUGH */
  CONNECT_REQ_INIT:
  case CONNECT_REQ_INIT:
    if(socks5_resolve_local) {
      enum resolve_t rc = Curl_resolv(data, hostname, remote_port,
                                      FALSE, &dns);

      if(rc == CURLRESOLV_ERROR)
        return CURLPX_RESOLVE_HOST;

      if(rc == CURLRESOLV_PENDING) {
        sxstate(data, CONNECT_RESOLVING);
        return CURLPX_OK;
      }
      sxstate(data, CONNECT_RESOLVED);
      goto CONNECT_RESOLVED;
    }
    goto CONNECT_RESOLVE_REMOTE;

  case CONNECT_RESOLVING:
    /* check if we have the name resolved by now */
    dns = Curl_fetch_addr(data, hostname, remote_port);

    if(dns) {
#ifdef CURLRES_ASYNCH
      data->state.async.dns = dns;
      data->state.async.done = TRUE;
#endif
      infof(data, "SOCKS5: hostname '%s' found", hostname);
    }

    if(!dns) {
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
    size_t destlen;
    if(dns)
      hp = dns->addr;
    if(!hp) {
      failf(data, "Failed to resolve \"%s\" for SOCKS5 connect.",
            hostname);
      return CURLPX_RESOLVE_HOST;
    }

    Curl_printable_address(hp, dest, sizeof(dest));
    destlen = strlen(dest);
    msnprintf(dest + destlen, sizeof(dest) - destlen, ":%d", remote_port);

    len = 0;
    socksreq[len++] = 5; /* version (SOCKS5) */
    socksreq[len++] = 1; /* connect */
    socksreq[len++] = 0; /* must be zero */
    if(hp->ai_family == AF_INET) {
      int i;
      struct sockaddr_in *saddr_in;
      socksreq[len++] = 1; /* ATYP: IPv4 = 1 */

      saddr_in = (struct sockaddr_in *)(void *)hp->ai_addr;
      for(i = 0; i < 4; i++) {
        socksreq[len++] = ((unsigned char *)&saddr_in->sin_addr.s_addr)[i];
      }

      infof(data, "SOCKS5 connect to IPv4 %s (locally resolved)", dest);
    }
