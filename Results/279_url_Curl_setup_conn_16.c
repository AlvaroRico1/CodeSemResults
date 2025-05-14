// Source: curl/lib/url.c
// Lines 4031-4035
CURLcode Curl_setup_conn(struct Curl_easy *data,
                         bool *protocol_done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  Curl_pgrsTime(data, TIMER_NAMELOOKUP);

  if(conn->handler->flags & PROTOPT_NONETWORK) {
    /* nothing to setup when not using a network */
    *protocol_done = TRUE;
    return result;
  }
  *protocol_done = FALSE; /* default to not done */

#ifndef CURL_DISABLE_PROXY
  /* set proxy_connect_closed to false unconditionally already here since it
     is used strictly to provide extra information to a parent function in the
     case of proxy CONNECT failures and we must make sure we don't have it
     lingering set from a previous invoke */
  conn->bits.proxy_connect_closed = FALSE;
#endif

#ifdef CURL_DO_LINEEND_CONV
  data->state.crlf_conversions = 0; /* reset CRLF conversion counter */
#endif /* CURL_DO_LINEEND_CONV */

  /* set start time here for timeout purposes in the connect procedure, it
     is later set again for the progress meter purpose */
  conn->now = Curl_now();

  if(CURL_SOCKET_BAD == conn->sock[FIRSTSOCKET]) {
    conn->bits.tcpconnect[FIRSTSOCKET] = FALSE;
    result = Curl_connecthost(data, conn, conn->dns_entry);
    if(result)
      return result;
  }
  else {
    Curl_pgrsTime(data, TIMER_CONNECT);    /* we're connected already */
    if(conn->ssl[FIRSTSOCKET].use ||
       (conn->handler->protocol & PROTO_FAMILY_SSH))
      Curl_pgrsTime(data, TIMER_APPCONNECT); /* we're connected already */
    conn->bits.tcpconnect[FIRSTSOCKET] = TRUE;
    *protocol_done = TRUE;
    Curl_updateconninfo(data, conn, conn->sock[FIRSTSOCKET]);
    Curl_verboseconnect(data, conn);
  }

  conn->now = Curl_now(); /* time this *after* the connect is done, we set
                             this here perhaps a second time */
  return result;
}
