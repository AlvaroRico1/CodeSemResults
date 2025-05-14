// Source: curl/lib/ftp.c
// Lines 930-934
static CURLcode ftp_state_use_port(struct Curl_easy *data,
                                   ftpport fcmd) /* start with this */
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  curl_socket_t portsock = CURL_SOCKET_BAD;
  char myhost[MAX_IPADR_LEN + 1] = "";

  struct Curl_sockaddr_storage ss;
  struct Curl_addrinfo *res, *ai;
  curl_socklen_t sslen;
  char hbuf[NI_MAXHOST];
  struct sockaddr *sa = (struct sockaddr *)&ss;
  struct sockaddr_in * const sa4 = (void *)sa;
#ifdef ENABLE_IPV6
  struct sockaddr_in6 * const sa6 = (void *)sa;
#endif
  static const char mode[][5] = { "EPRT", "PORT" };
  enum resolve_t rc;
  int error;
  char *host = NULL;
  char *string_ftpport = data->set.str[STRING_FTPPORT];
  struct Curl_dns_entry *h = NULL;
  unsigned short port_min = 0;
  unsigned short port_max = 0;
  unsigned short port;
  bool possibly_non_local = TRUE;
  char buffer[STRERROR_LEN];
  char *addr = NULL;

  /* Step 1, figure out what is requested,
   * accepted format :
   * (ipv4|ipv6|domain|interface)?(:port(-range)?)?
   */

  if(data->set.str[STRING_FTPPORT] &&
     (strlen(data->set.str[STRING_FTPPORT]) > 1)) {

#ifdef ENABLE_IPV6
    size_t addrlen = INET6_ADDRSTRLEN > strlen(string_ftpport) ?
      INET6_ADDRSTRLEN : strlen(string_ftpport);
#else
    size_t addrlen = INET_ADDRSTRLEN > strlen(string_ftpport) ?
      INET_ADDRSTRLEN : strlen(string_ftpport);
#endif
    char *ip_start = string_ftpport;
    char *ip_end = NULL;
    char *port_start = NULL;
    char *port_sep = NULL;

    addr = calloc(addrlen + 1, 1);
    if(!addr)
      return CURLE_OUT_OF_MEMORY;

#ifdef ENABLE_IPV6
    if(*string_ftpport == '[') {
      /* [ipv6]:port(-range) */
      ip_start = string_ftpport + 1;
      ip_end = strchr(string_ftpport, ']');
      if(ip_end)
        strncpy(addr, ip_start, ip_end - ip_start);
    }
    else
#endif
      if(*string_ftpport == ':') {
        /* :port */
        ip_end = string_ftpport;
      }
      else {
        ip_end = strchr(string_ftpport, ':');
        if(ip_end) {
          /* either ipv6 or (ipv4|domain|interface):port(-range) */
#ifdef ENABLE_IPV6
          if(Curl_inet_pton(AF_INET6, string_ftpport, sa6) == 1) {
            /* ipv6 */
            port_min = port_max = 0;
            strcpy(addr, string_ftpport);
            ip_end = NULL; /* this got no port ! */
          }
          else
#endif
            /* (ipv4|domain|interface):port(-range) */
            strncpy(addr, string_ftpport, ip_end - ip_start);
        }
        else
          /* ipv4|interface */
          strcpy(addr, string_ftpport);
      }

    /* parse the port */
    if(ip_end != NULL) {
      port_start = strchr(ip_end, ':');
      if(port_start) {
        port_min = curlx_ultous(strtoul(port_start + 1, NULL, 10));
        port_sep = strchr(port_start, '-');
        if(port_sep) {
          port_max = curlx_ultous(strtoul(port_sep + 1, NULL, 10));
        }
        else
          port_max = port_min;
      }
    }

    /* correct errors like:
     *  :1234-1230
     *  :-4711,  in this case port_min is (unsigned)-1,
     *           therefore port_min > port_max for all cases
     *           but port_max = (unsigned)-1
     */
    if(port_min > port_max)
      port_min = port_max = 0;


    if(*addr != '\0') {
      /* attempt to get the address of the given interface name */
      switch(Curl_if2ip(conn->ip_addr->ai_family,
                        Curl_ipv6_scope(conn->ip_addr->ai_addr),
                        conn->scope_id, addr, hbuf, sizeof(hbuf))) {
        case IF2IP_NOT_FOUND:
          /* not an interface, use the given string as host name instead */
          host = addr;
          break;
        case IF2IP_AF_NOT_SUPPORTED:
          return CURLE_FTP_PORT_FAILED;
        case IF2IP_FOUND:
          host = hbuf; /* use the hbuf for host name */
      }
    }
    else
      /* there was only a port(-range) given, default the host */
      host = NULL;
  } /* data->set.ftpport */

  if(!host) {
    const char *r;
    /* not an interface and not a host name, get default by extracting
       the IP from the control connection */
    sslen = sizeof(ss);
    if(getsockname(conn->sock[FIRSTSOCKET], sa, &sslen)) {
      failf(data, "getsockname() failed: %s",
            Curl_strerror(SOCKERRNO, buffer, sizeof(buffer)));
      free(addr);
      return CURLE_FTP_PORT_FAILED;
    }
    switch(sa->sa_family) {
#ifdef ENABLE_IPV6
    case AF_INET6:
      r = Curl_inet_ntop(sa->sa_family, &sa6->sin6_addr, hbuf, sizeof(hbuf));
      break;
#endif
    default:
      r = Curl_inet_ntop(sa->sa_family, &sa4->sin_addr, hbuf, sizeof(hbuf));
      break;
    }
    if(!r)
      return CURLE_FTP_PORT_FAILED;
    host = hbuf; /* use this host name */
    possibly_non_local = FALSE; /* we know it is local now */
  }

  /* resolv ip/host to ip */
  rc = Curl_resolv(data, host, 0, FALSE, &h);
  if(rc == CURLRESOLV_PENDING)
    (void)Curl_resolver_wait_resolv(data, &h);
  if(h) {
    res = h->addr;
    /* when we return from this function, we can forget about this entry
       to we can unlock it now already */
    Curl_resolv_unlock(data, h);
  } /* (h) */
  else
    res = NULL; /* failure! */

  if(!res) {
    failf(data, "failed to resolve the address provided to PORT: %s", host);
    free(addr);
    return CURLE_FTP_PORT_FAILED;
  }

  free(addr);
  host = NULL;

  /* step 2, create a socket for the requested address */

  portsock = CURL_SOCKET_BAD;
  error = 0;
  for(ai = res; ai; ai = ai->ai_next) {
    result = Curl_socket(data, ai, NULL, &portsock);
    if(result) {
      error = SOCKERRNO;
      continue;
    }
    break;
  }
  if(!ai) {
    failf(data, "socket failure: %s",
          Curl_strerror(error, buffer, sizeof(buffer)));
    return CURLE_FTP_PORT_FAILED;
  }

  /* step 3, bind to a suitable local address */

  memcpy(sa, ai->ai_addr, ai->ai_addrlen);
  sslen = ai->ai_addrlen;

  for(port = port_min; port <= port_max;) {
    if(sa->sa_family == AF_INET)
      sa4->sin_port = htons(port);
#ifdef ENABLE_IPV6
    else
      sa6->sin6_port = htons(port);
#endif
    /* Try binding the given address. */
    if(bind(portsock, sa, sslen) ) {
      /* It failed. */
      error = SOCKERRNO;
      if(possibly_non_local && (error == EADDRNOTAVAIL)) {
        /* The requested bind address is not local.  Use the address used for
         * the control connection instead and restart the port loop
         */
        infof(data, "bind(port=%hu) on non-local address failed: %s", port,
              Curl_strerror(error, buffer, sizeof(buffer)));

        sslen = sizeof(ss);
        if(getsockname(conn->sock[FIRSTSOCKET], sa, &sslen)) {
          failf(data, "getsockname() failed: %s",
                Curl_strerror(SOCKERRNO, buffer, sizeof(buffer)));
          Curl_closesocket(data, conn, portsock);
          return CURLE_FTP_PORT_FAILED;
        }
        port = port_min;
        possibly_non_local = FALSE; /* don't try this again */
        continue;
      }
      if(error != EADDRINUSE && error != EACCES) {
        failf(data, "bind(port=%hu) failed: %s", port,
              Curl_strerror(error, buffer, sizeof(buffer)));
        Curl_closesocket(data, conn, portsock);
        return CURLE_FTP_PORT_FAILED;
      }
    }
    else
      break;

    port++;
  }

  /* maybe all ports were in use already*/
  if(port > port_max) {
    failf(data, "bind() failed, we ran out of ports!");
    Curl_closesocket(data, conn, portsock);
    return CURLE_FTP_PORT_FAILED;
  }

  /* get the name again after the bind() so that we can extract the
     port number it uses now */
  sslen = sizeof(ss);
  if(getsockname(portsock, (struct sockaddr *)sa, &sslen)) {
    failf(data, "getsockname() failed: %s",
          Curl_strerror(SOCKERRNO, buffer, sizeof(buffer)));
    Curl_closesocket(data, conn, portsock);
    return CURLE_FTP_PORT_FAILED;
  }

  /* step 4, listen on the socket */

  if(listen(portsock, 1)) {
    failf(data, "socket failure: %s",
          Curl_strerror(SOCKERRNO, buffer, sizeof(buffer)));
    Curl_closesocket(data, conn, portsock);
    return CURLE_FTP_PORT_FAILED;
  }

  /* step 5, send the proper FTP command */

  /* get a plain printable version of the numerical address to work with
     below */
  Curl_printable_address(ai, myhost, sizeof(myhost));

#ifdef ENABLE_IPV6
  if(!conn->bits.ftp_use_eprt && conn->bits.ipv6)
    /* EPRT is disabled but we are connected to a IPv6 host, so we ignore the
       request and enable EPRT again! */
    conn->bits.ftp_use_eprt = TRUE;
#endif

  for(; fcmd != DONE; fcmd++) {

    if(!conn->bits.ftp_use_eprt && (EPRT == fcmd))
      /* if disabled, goto next */
      continue;

    if((PORT == fcmd) && sa->sa_family != AF_INET)
      /* PORT is IPv4 only */
      continue;

    switch(sa->sa_family) {
    case AF_INET:
      port = ntohs(sa4->sin_port);
      break;
#ifdef ENABLE_IPV6
    case AF_INET6:
      port = ntohs(sa6->sin6_port);
      break;
#endif
    default:
      continue; /* might as well skip this */
    }

    if(EPRT == fcmd) {
      /*
       * Two fine examples from RFC2428;
       *
       * EPRT |1|132.235.1.2|6275|
       *
       * EPRT |2|1080::8:800:200C:417A|5282|
       */

      result = Curl_pp_sendf(data, &ftpc->pp, "%s |%d|%s|%hu|", mode[fcmd],
                             sa->sa_family == AF_INET?1:2,
                             myhost, port);
      if(result) {
        failf(data, "Failure sending EPRT command: %s",
              curl_easy_strerror(result));
        Curl_closesocket(data, conn, portsock);
        /* don't retry using PORT */
        ftpc->count1 = PORT;
        /* bail out */
        state(data, FTP_STOP);
        return result;
      }
      break;
    }
    if(PORT == fcmd) {
      /* large enough for [IP address],[num],[num] */
      char target[sizeof(myhost) + 20];
      char *source = myhost;
      char *dest = target;

      /* translate x.x.x.x to x,x,x,x */
      while(source && *source) {
        if(*source == '.')
          *dest = ',';
        else
          *dest = *source;
        dest++;
        source++;
      }
      *dest = 0;
      msnprintf(dest, 20, ",%d,%d", (int)(port>>8), (int)(port&0xff));

      result = Curl_pp_sendf(data, &ftpc->pp, "%s %s", mode[fcmd], target);
      if(result) {
        failf(data, "Failure sending PORT command: %s",
              curl_easy_strerror(result));
        Curl_closesocket(data, conn, portsock);
        /* bail out */
        state(data, FTP_STOP);
        return result;
      }
      break;
    }
  }

  /* store which command was sent */
  ftpc->count1 = fcmd;

  close_secondarysocket(data, conn);

  /* we set the secondary socket variable to this for now, it is only so that
     the cleanup function will close it in case we fail before the true
     secondary stuff is made */
  conn->sock[SECONDARYSOCKET] = portsock;

  /* this tcpconnect assignment below is a hackish work-around to make the
     multi interface with active FTP work - as it will not wait for a
     (passive) connect in Curl_is_connected().

     The *proper* fix is to make sure that the active connection from the
     server is done in a non-blocking way. Currently, it is still BLOCKING.
  */
  conn->bits.tcpconnect[SECONDARYSOCKET] = TRUE;

  state(data, FTP_PORT);
  return result;
}
