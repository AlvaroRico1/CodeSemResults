static CURLMcode multi_runsingle(struct Curl_multi *multi,
                                 struct curltime *nowp,
                                 struct Curl_easy *data)
{
  struct Curl_message *msg = NULL;
  bool connected;
  bool async;
  bool protocol_connected = FALSE;
  bool dophase_done = FALSE;
  bool done = FALSE;
  CURLMcode rc;
  CURLcode result = CURLE_OK;
  timediff_t recv_timeout_ms;
  timediff_t send_timeout_ms;
  int control;

  if(!GOOD_EASY_HANDLE(data))
    return CURLM_BAD_EASY_HANDLE;

  do {
    /* A "stream" here is a logical stream if the protocol can handle that
       (HTTP/2), or the full connection for older protocols */
    bool stream_error = FALSE;
    rc = CURLM_OK;

    if(multi_ischanged(multi, TRUE)) {
      DEBUGF(infof(data, "multi changed, check CONNECT_PEND queue!"));
      process_pending_handles(multi); /* multiplexed */
    }

    if(data->mstate > MSTATE_CONNECT &&
       data->mstate < MSTATE_COMPLETED) {
      /* Make sure we set the connection's current owner */
      DEBUGASSERT(data->conn);
      if(!data->conn)
        return CURLM_INTERNAL_ERROR;
    }

    if(data->conn &&
       (data->mstate >= MSTATE_CONNECT) &&
       (data->mstate < MSTATE_COMPLETED)) {
      /* Check for overall operation timeout here but defer handling the
       * connection timeout to later, to allow for a connection to be set up
       * in the window since we last checked timeout. This prevents us
       * tearing down a completed connection in the case where we were slow
       * to check the timeout (e.g. process descheduled during this loop).
       * We set connect_timeout=FALSE to do this. */

      /* we need to wait for the connect state as only then is the start time
         stored, but we must not check already completed handles */
      if(multi_handle_timeout(data, nowp, &stream_error, &result, FALSE)) {
        /* Skip the statemachine and go directly to error handling section. */
        goto statemachine_end;
      }
    }

    switch(data->mstate) {
    case MSTATE_INIT:
      /* init this transfer. */
      result = Curl_pretransfer(data);

      if(!result) {
        /* after init, go CONNECT */
        multistate(data, MSTATE_CONNECT);
        *nowp = Curl_pgrsTime(data, TIMER_STARTOP);
        rc = CURLM_CALL_MULTI_PERFORM;
      }
      break;

    case MSTATE_PENDING:
      /* We will stay here until there is a connection available. Then
         we try again in the MSTATE_CONNECT state. */
      break;

    case MSTATE_CONNECT:
      /* Connect. We want to get a connection identifier filled in. */
      /* init this transfer. */
      result = Curl_preconnect(data);
      if(result)
        break;

      *nowp = Curl_pgrsTime(data, TIMER_STARTSINGLE);
      if(data->set.timeout)
        Curl_expire(data, data->set.timeout, EXPIRE_TIMEOUT);

      if(data->set.connecttimeout)
        Curl_expire(data, data->set.connecttimeout, EXPIRE_CONNECTTIMEOUT);

      result = Curl_connect(data, &async, &protocol_connected);
      if(CURLE_NO_CONNECTION_AVAILABLE == result) {
        /* There was no connection available. We will go to the pending
           state and wait for an available connection. */
        multistate(data, MSTATE_PENDING);

        /* add this handle to the list of connect-pending handles */
        Curl_llist_insert_next(&multi->pending, multi->pending.tail, data,
                               &data->connect_queue);
        result = CURLE_OK;
        break;
      }
      else if(data->state.previouslypending) {
        /* this transfer comes from the pending queue so try move another */
        infof(data, "Transfer was pending, now try another");
        process_pending_handles(data->multi);
      }

      if(!result) {
        if(async)
          /* We're now waiting for an asynchronous name lookup */
          multistate(data, MSTATE_RESOLVING);
        else {
          /* after the connect has been sent off, go WAITCONNECT unless the
             protocol connect is already done and we can go directly to
             WAITDO or DO! */
          rc = CURLM_CALL_MULTI_PERFORM;

          if(protocol_connected)
            multistate(data, MSTATE_DO);
          else {
#ifndef CURL_DISABLE_HTTP
            if(Curl_connect_ongoing(data->conn))
              multistate(data, MSTATE_TUNNELING);
            else
#endif
              multistate(data, MSTATE_CONNECTING);
          }
        }
      }
      break;

    case MSTATE_RESOLVING:
      /* awaiting an asynch name resolve to complete */
    {
      struct Curl_dns_entry *dns = NULL;
      struct connectdata *conn = data->conn;
      const char *hostname;

      DEBUGASSERT(conn);
#ifndef CURL_DISABLE_PROXY
      if(conn->bits.httpproxy)
        hostname = conn->http_proxy.host.name;
      else
#endif
        if(conn->bits.conn_to_host)
        hostname = conn->conn_to_host.name;
      else
        hostname = conn->host.name;

      /* check if we have the name resolved by now */
      dns = Curl_fetch_addr(data, hostname, (int)conn->port);

      if(dns) {
#ifdef CURLRES_ASYNCH
        data->state.async.dns = dns;
        data->state.async.done = TRUE;
#endif
        result = CURLE_OK;
        infof(data, "Hostname '%s' was found in DNS cache", hostname);
      }

      if(!dns)
        result = Curl_resolv_check(data, &dns);

      /* Update sockets here, because the socket(s) may have been
         closed and the application thus needs to be told, even if it
         is likely that the same socket(s) will again be used further
         down.  If the name has not yet been resolved, it is likely
         that new sockets have been opened in an attempt to contact
         another resolver. */
      singlesocket(multi, data);

      if(dns) {
        /* Perform the next step in the connection phase, and then move on
           to the WAITCONNECT state */
        result = Curl_once_resolved(data, &protocol_connected);

        if(result)
          /* if Curl_once_resolved() returns failure, the connection struct
             is already freed and gone */
          data->conn = NULL; /* no more connection */
        else {
          /* call again please so that we get the next socket setup */
          rc = CURLM_CALL_MULTI_PERFORM;
          if(protocol_connected)
            multistate(data, MSTATE_DO);
          else {
#ifndef CURL_DISABLE_HTTP
            if(Curl_connect_ongoing(data->conn))
              multistate(data, MSTATE_TUNNELING);
            else
#endif
              multistate(data, MSTATE_CONNECTING);
          }
        }
      }


// Source: multi.c
// Lines 1720-1914
