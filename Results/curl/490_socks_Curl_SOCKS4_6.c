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
      else
        failf(data, "SOCKS4 connection to %s not supported", hostname);
    }
    else
      failf(data, "Failed to resolve \"%s\" for SOCKS4 connect.",
            hostname);

    if(!hp)
      return CURLPX_RESOLVE_HOST;
  }
    /* FALLTHROUGH */
  CONNECT_REQ_INIT:
  case CONNECT_REQ_INIT:
    /*
     * This is currently not supporting "Identification Protocol (RFC1413)".
     */
    socksreq[8] = 0; /* ensure empty userid is NUL-terminated */
    if(proxy_user) {
      size_t plen = strlen(proxy_user);
      if(plen >= (size_t)data->set.buffer_size - 8) {
        failf(data, "Too long SOCKS proxy user name, can't use!");
        return CURLPX_LONG_USER;
      }
      /* copy the proxy name WITH trailing zero */
      memcpy(socksreq + 8, proxy_user, plen + 1);
    }

    /*
     * Make connection
     */
    {
      size_t packetsize = 9 +
        strlen((char *)socksreq + 8); /* size including NUL */

      /* If SOCKS4a, set special invalid IP address 0.0.0.x */
      if(protocol4a) {
        size_t hostnamelen = 0;
        socksreq[4] = 0;
        socksreq[5] = 0;
        socksreq[6] = 0;
        socksreq[7] = 1;
        /* append hostname */
        hostnamelen = strlen(hostname) + 1; /* length including NUL */
        if(hostnamelen <= 255)
          strcpy((char *)socksreq + packetsize, hostname);
        else {
          failf(data, "SOCKS4: too long host name");
          return CURLPX_LONG_HOSTNAME;
        }
        packetsize += hostnamelen;
      }
      sx->outp = socksreq;
      sx->outstanding = packetsize;
      sxstate(data, CONNECT_REQ_SENDING);
    }
    /* FALLTHROUGH */
  case CONNECT_REQ_SENDING:
    /* Send request */
    result = Curl_write_plain(data, sockfd, (char *)sx->outp,
                              sx->outstanding, &written);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "Failed to send SOCKS4 connect request.");
      return CURLPX_SEND_CONNECT;
    }
    if(written != sx->outstanding) {
      /* not done, remain in state */
      sx->outstanding -= written;
      sx->outp += written;
      return CURLPX_OK;
    }

    /* done sending! */
    sx->outstanding = 8; /* receive data size */
    sx->outp = socksreq;
    sxstate(data, CONNECT_SOCKS_READ);

    /* FALLTHROUGH */
  case CONNECT_SOCKS_READ:
    /* Receive response */
    result = Curl_read_plain(sockfd, (char *)sx->outp,
                             sx->outstanding, &actualread);
    if(result && (CURLE_AGAIN != result)) {
      failf(data, "SOCKS4: Failed receiving connect request ack: %s",
            curl_easy_strerror(result));
      return CURLPX_RECV_CONNECT;
    }
    else if(!result && !actualread) {
      /* connection closed */
      failf(data, "connection to proxy closed");
      return CURLPX_CLOSED;
    }
    else if(actualread != sx->outstanding) {
      /* remain in reading state */
      sx->outstanding -= actualread;
      sx->outp += actualread;
      return CURLPX_OK;
    }
    sxstate(data, CONNECT_DONE);
    break;
  default: /* lots of unused states in SOCKS4 */
    break;
  }

  /*
   * Response format
   *
   *     +----+----+----+----+----+----+----+----+
   *     | VN | CD | DSTPORT |      DSTIP        |
   *     +----+----+----+----+----+----+----+----+
   * # of bytes:  1    1      2              4
   *
   * VN is the version of the reply code and should be 0. CD is the result
   * code with one of the following values:
   *
   * 90: request granted
   * 91: request rejected or failed
   * 92: request rejected because SOCKS server cannot connect to
   *     identd on the client
   * 93: request rejected because the client program and identd
   *     report different user-ids
   */

  /* wrong version ? */
  if(socksreq[0]) {
    failf(data,
          "SOCKS4 reply has wrong version, version should be 0.");
    return CURLPX_BAD_VERSION;
  }

  /* Result */
  switch(socksreq[1]) {
  case 90:
    infof(data, "SOCKS4%s request granted.", protocol4a?"a":"");
    break;
  case 91:
    failf(data,
          "Can't complete SOCKS4 connection to %d.%d.%d.%d:%d. (%d)"
          ", request rejected or failed.",
          socksreq[4], socksreq[5], socksreq[6], socksreq[7],
          (((unsigned char)socksreq[2] << 8) | (unsigned char)socksreq[3]),
          (unsigned char)socksreq[1]);
    return CURLPX_REQUEST_FAILED;
  case 92:
    failf(data,
          "Can't complete SOCKS4 connection to %d.%d.%d.%d:%d. (%d)"
          ", request rejected because SOCKS server cannot connect to "
          "identd on the client.",
          socksreq[4], socksreq[5], socksreq[6], socksreq[7],
          (((unsigned char)socksreq[2] << 8) | (unsigned char)socksreq[3]),
          (unsigned char)socksreq[1]);
    return CURLPX_IDENTD;
  case 93:
    failf(data,
          "Can't complete SOCKS4 connection to %d.%d.%d.%d:%d. (%d)"
          ", request rejected because the client program and identd "
          "report different user-ids.",
          socksreq[4], socksreq[5], socksreq[6], socksreq[7],
          (((unsigned char)socksreq[2] << 8) | (unsigned char)socksreq[3]),
          (unsigned char)socksreq[1]);
    return CURLPX_IDENTD_DIFFER;
  default:
    failf(data,
          "Can't complete SOCKS4 connection to %d.%d.%d.%d:%d. (%d)"
          ", Unknown.",
          socksreq[4], socksreq[5], socksreq[6], socksreq[7],
          (((unsigned char)socksreq[2] << 8) | (unsigned char)socksreq[3]),
          (unsigned char)socksreq[1]);
    return CURLPX_UNKNOWN_FAIL;
  }

  *done = TRUE;
  return CURLPX_OK; /* Proxy was successful! */
}


// Source: socks.c
// Lines 188-480
