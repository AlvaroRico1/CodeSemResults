static CURLcode ftp_state_pasv_resp(struct Curl_easy *data,
                                    int ftpcode)
{
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  CURLcode result;
  struct Curl_dns_entry *addr = NULL;
  enum resolve_t rc;
  unsigned short connectport; /* the local port connect() should use! */
  char *str = &data->state.buffer[4];  /* start on the first letter */

  /* if we come here again, make sure the former name is cleared */
  Curl_safefree(ftpc->newhost);

  if((ftpc->count1 == 0) &&
     (ftpcode == 229)) {
    /* positive EPSV response */
    char *ptr = strchr(str, '(');
    if(ptr) {
      unsigned int num;
      char separator[4];
      ptr++;
      if(5 == sscanf(ptr, "%c%c%c%u%c",
                     &separator[0],
                     &separator[1],
                     &separator[2],
                     &num,
                     &separator[3])) {
        const char sep1 = separator[0];
        int i;

        /* The four separators should be identical, or else this is an oddly
           formatted reply and we bail out immediately. */
        for(i = 1; i<4; i++) {
          if(separator[i] != sep1) {
            ptr = NULL; /* set to NULL to signal error */
            break;
          }
        }
        if(num > 0xffff) {
          failf(data, "Illegal port number in EPSV reply");
          return CURLE_FTP_WEIRD_PASV_REPLY;
        }
        if(ptr) {
          ftpc->newport = (unsigned short)(num & 0xffff);
          ftpc->newhost = strdup(control_address(conn));
          if(!ftpc->newhost)
            return CURLE_OUT_OF_MEMORY;
        }
      }
      else
        ptr = NULL;
    }
    if(!ptr) {
      failf(data, "Weirdly formatted EPSV reply");
      return CURLE_FTP_WEIRD_PASV_REPLY;
    }
  }
  else if((ftpc->count1 == 1) &&
          (ftpcode == 227)) {
    /* positive PASV response */
    unsigned int ip[4] = {0, 0, 0, 0};
    unsigned int port[2] = {0, 0};

    /*
     * Scan for a sequence of six comma-separated numbers and use them as
     * IP+port indicators.
     *
     * Found reply-strings include:
     * "227 Entering Passive Mode (127,0,0,1,4,51)"
     * "227 Data transfer will passively listen to 127,0,0,1,4,51"
     * "227 Entering passive mode. 127,0,0,1,4,51"
     */
    while(*str) {
      if(6 == sscanf(str, "%u,%u,%u,%u,%u,%u",
                     &ip[0], &ip[1], &ip[2], &ip[3],
                     &port[0], &port[1]))
        break;
      str++;
    }

    if(!*str || (ip[0] > 255) || (ip[1] > 255)  || (ip[2] > 255)  ||
       (ip[3] > 255) || (port[0] > 255)  || (port[1] > 255) ) {
      failf(data, "Couldn't interpret the 227-response");
      return CURLE_FTP_WEIRD_227_FORMAT;
    }

    /* we got OK from server */
    if(data->set.ftp_skip_ip) {
      /* told to ignore the remotely given IP but instead use the host we used
         for the control connection */
      infof(data, "Skip %u.%u.%u.%u for data connection, re-use %s instead",
            ip[0], ip[1], ip[2], ip[3],
            conn->host.name);
      ftpc->newhost = strdup(control_address(conn));
    }
    else
      ftpc->newhost = aprintf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

    if(!ftpc->newhost)
      return CURLE_OUT_OF_MEMORY;

    ftpc->newport = (unsigned short)(((port[0]<<8) + port[1]) & 0xffff);
  }
  else if(ftpc->count1 == 0) {
    /* EPSV failed, move on to PASV */
    return ftp_epsv_disable(data, conn);
  }
  else {
    failf(data, "Bad PASV/EPSV response: %03d", ftpcode);
    return CURLE_FTP_WEIRD_PASV_REPLY;
  }

#ifndef CURL_DISABLE_PROXY
  if(conn->bits.proxy) {
    /*
     * This connection uses a proxy and we need to connect to the proxy again
     * here. We don't want to rely on a former host lookup that might've
     * expired now, instead we remake the lookup here and now!
     */
    const char * const host_name = conn->bits.socksproxy ?
      conn->socks_proxy.host.name : conn->http_proxy.host.name;
    rc = Curl_resolv(data, host_name, (int)conn->port, FALSE, &addr);
    if(rc == CURLRESOLV_PENDING)
      /* BLOCKING, ignores the return code but 'addr' will be NULL in
         case of failure */
      (void)Curl_resolver_wait_resolv(data, &addr);

    connectport =
      (unsigned short)conn->port; /* we connect to the proxy's port */

    if(!addr) {
      failf(data, "Can't resolve proxy host %s:%hu", host_name, connectport);
      return CURLE_COULDNT_RESOLVE_PROXY;
    }
  }
  else
#endif
  {
    /* normal, direct, ftp connection */
    DEBUGASSERT(ftpc->newhost);

    /* postponed address resolution in case of tcp fastopen */
    if(conn->bits.tcp_fastopen && !conn->bits.reuse && !ftpc->newhost[0]) {
      Curl_conninfo_remote(data, conn, conn->sock[FIRSTSOCKET]);
      Curl_safefree(ftpc->newhost);
      ftpc->newhost = strdup(control_address(conn));
      if(!ftpc->newhost)
        return CURLE_OUT_OF_MEMORY;
    }

    rc = Curl_resolv(data, ftpc->newhost, ftpc->newport, FALSE, &addr);
    if(rc == CURLRESOLV_PENDING)
      /* BLOCKING */
      (void)Curl_resolver_wait_resolv(data, &addr);

    connectport = ftpc->newport; /* we connect to the remote port */

    if(!addr) {
      failf(data, "Can't resolve new host %s:%hu", ftpc->newhost, connectport);
      return CURLE_FTP_CANT_GET_HOST;
    }
  }

  conn->bits.tcpconnect[SECONDARYSOCKET] = FALSE;
  result = Curl_connecthost(data, conn, addr);

  if(result) {
    Curl_resolv_unlock(data, addr); /* we're done using this address */
    if(ftpc->count1 == 0 && ftpcode == 229)
      return ftp_epsv_disable(data, conn);

    return result;
  }


  /*
   * When this is used from the multi interface, this might've returned with
   * the 'connected' set to FALSE and thus we are now awaiting a non-blocking
   * connect to connect.
   */

  if(data->set.verbose)
    /* this just dumps information about this second connection */
    ftp_pasv_verbose(data, addr->addr, ftpc->newhost, connectport);

  Curl_resolv_unlock(data, addr); /* we're done using this address */

  Curl_safefree(conn->secondaryhostname);
  conn->secondary_port = ftpc->newport;
  conn->secondaryhostname = strdup(ftpc->newhost);
  if(!conn->secondaryhostname)
    return CURLE_OUT_OF_MEMORY;

  conn->bits.do_more = TRUE;
  state(data, FTP_STOP); /* this phase is completed */

  return result;
}


// Source: ftp.c
// Lines 1833-2031
