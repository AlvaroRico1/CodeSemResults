// Source: curl/lib/connect.c
// Lines 938-938
CURLcode Curl_is_connected(struct Curl_easy *data,
                           struct connectdata *conn,
                           int sockindex,
                           bool *connected)
{
  CURLcode result = CURLE_OK;
  timediff_t allow;
  int error = 0;
  struct curltime now;
  int rc = 0;
  unsigned int i;

  DEBUGASSERT(sockindex >= FIRSTSOCKET && sockindex <= SECONDARYSOCKET);

  *connected = FALSE; /* a very negative world view is best */

  if(conn->bits.tcpconnect[sockindex]) {
    /* we are connected already! */
    *connected = TRUE;
    return CURLE_OK;
  }

  now = Curl_now();

  if(SOCKS_STATE(conn->cnnct.state)) {
    /* still doing SOCKS */
    result = connect_SOCKS(data, sockindex, connected);
    if(!result && *connected)
      post_SOCKS(data, conn, sockindex, connected);
    return result;
  }

  for(i = 0; i<2; i++) {
    const int other = i ^ 1;
    if(conn->tempsock[i] == CURL_SOCKET_BAD)
      continue;
    error = 0;
#ifdef ENABLE_QUIC
    if(conn->transport == TRNSPRT_QUIC) {
      result = Curl_quic_is_connected(data, conn, i, connected);
      if(!result && *connected) {
        /* use this socket from now on */
        conn->sock[sockindex] = conn->tempsock[i];
        conn->ip_addr = conn->tempaddr[i];
        conn->tempsock[i] = CURL_SOCKET_BAD;
        post_SOCKS(data, conn, sockindex, connected);
        connkeep(conn, "HTTP/3 default");
        return CURLE_OK;
      }
      if(result) {
        conn->tempsock[i] = CURL_SOCKET_BAD;
        error = SOCKERRNO;
      }
    }
    else
#endif
    {
#ifdef mpeix
      /* Call this function once now, and ignore the results. We do this to
         "clear" the error state on the socket so that we can later read it
         reliably. This is reported necessary on the MPE/iX operating
         system. */
      (void)verifyconnect(conn->tempsock[i], NULL);
#endif

      /* check socket for connect */
      rc = SOCKET_WRITABLE(conn->tempsock[i], 0);
    }

    if(rc == 0) { /* no connection yet */
      if(Curl_timediff(now, conn->connecttime) >=
         conn->timeoutms_per_addr[i]) {
        infof(data, "After %" CURL_FORMAT_TIMEDIFF_T
              "ms connect time, move on!", conn->timeoutms_per_addr[i]);
        error = ETIMEDOUT;
      }

      /* should we try another protocol family? */
      if(i == 0 && !conn->bits.parallel_connect &&
         (Curl_timediff(now, conn->connecttime) >=
          data->set.happy_eyeballs_timeout)) {
        conn->bits.parallel_connect = TRUE; /* starting now */
        trynextip(data, conn, sockindex, 1);
      }
    }
    else if(rc == CURL_CSELECT_OUT || conn->bits.tcp_fastopen) {
      if(verifyconnect(conn->tempsock[i], &error)) {
        /* we are connected with TCP, awesome! */

        /* use this socket from now on */
        conn->sock[sockindex] = conn->tempsock[i];
        conn->ip_addr = conn->tempaddr[i];
        conn->tempsock[i] = CURL_SOCKET_BAD;
#ifdef ENABLE_IPV6
        conn->bits.ipv6 = (conn->ip_addr->ai_family == AF_INET6)?TRUE:FALSE;
#endif

        /* close the other socket, if open */
        if(conn->tempsock[other] != CURL_SOCKET_BAD) {
          Curl_closesocket(data, conn, conn->tempsock[other]);
          conn->tempsock[other] = CURL_SOCKET_BAD;
        }

        /* see if we need to kick off any SOCKS proxy magic once we
           connected */
        result = connect_SOCKS(data, sockindex, connected);
        if(result || !*connected)
          return result;

        post_SOCKS(data, conn, sockindex, connected);

        return CURLE_OK;
      }
