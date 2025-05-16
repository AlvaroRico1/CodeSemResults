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

      if(result) {
        /* failure detected */
        stream_error = TRUE;
        break;
      }
    }
    break;

#ifndef CURL_DISABLE_HTTP
    case MSTATE_TUNNELING:
      /* this is HTTP-specific, but sending CONNECT to a proxy is HTTP... */
      DEBUGASSERT(data->conn);
      result = Curl_http_connect(data, &protocol_connected);
#ifndef CURL_DISABLE_PROXY
      if(data->conn->bits.proxy_connect_closed) {
        rc = CURLM_CALL_MULTI_PERFORM;
        /* connect back to proxy again */
        result = CURLE_OK;
        multi_done(data, CURLE_OK, FALSE);
        multistate(data, MSTATE_CONNECT);
      }
      else
#endif
        if(!result) {
          if(
#ifndef CURL_DISABLE_PROXY
            (data->conn->http_proxy.proxytype != CURLPROXY_HTTPS ||
             data->conn->bits.proxy_ssl_connected[FIRSTSOCKET]) &&
#endif
            Curl_connect_complete(data->conn)) {
            rc = CURLM_CALL_MULTI_PERFORM;
            /* initiate protocol connect phase */
            multistate(data, MSTATE_PROTOCONNECT);
          }
        }
      else
        stream_error = TRUE;
      break;
#endif

    case MSTATE_CONNECTING:
      /* awaiting a completion of an asynch TCP connect */
      DEBUGASSERT(data->conn);
      result = Curl_is_connected(data, data->conn, FIRSTSOCKET, &connected);
      if(connected && !result) {
#ifndef CURL_DISABLE_HTTP
        if(
#ifndef CURL_DISABLE_PROXY
          (data->conn->http_proxy.proxytype == CURLPROXY_HTTPS &&
           !data->conn->bits.proxy_ssl_connected[FIRSTSOCKET]) ||
#endif
          Curl_connect_ongoing(data->conn)) {
          multistate(data, MSTATE_TUNNELING);
          break;
        }
#endif
        rc = CURLM_CALL_MULTI_PERFORM;
#ifndef CURL_DISABLE_PROXY
        multistate(data,
                   data->conn->bits.tunnel_proxy?
                   MSTATE_TUNNELING : MSTATE_PROTOCONNECT);
#else
        multistate(data, MSTATE_PROTOCONNECT);
#endif
      }
      else if(result) {
        /* failure detected */
        Curl_posttransfer(data);
        multi_done(data, result, TRUE);
        stream_error = TRUE;
        break;
      }
      break;

    case MSTATE_PROTOCONNECT:
      result = protocol_connect(data, &protocol_connected);
      if(!result && !protocol_connected)
        /* switch to waiting state */
        multistate(data, MSTATE_PROTOCONNECTING);
      else if(!result) {
        /* protocol connect has completed, go WAITDO or DO */
        multistate(data, MSTATE_DO);
        rc = CURLM_CALL_MULTI_PERFORM;
      }
      else {
        /* failure detected */
        Curl_posttransfer(data);
        multi_done(data, result, TRUE);
        stream_error = TRUE;
      }
      break;

    case MSTATE_PROTOCONNECTING:
      /* protocol-specific connect phase */
      result = protocol_connecting(data, &protocol_connected);
      if(!result && protocol_connected) {
        /* after the connect has completed, go WAITDO or DO */
        multistate(data, MSTATE_DO);
        rc = CURLM_CALL_MULTI_PERFORM;
      }
      else if(result) {
        /* failure detected */
        Curl_posttransfer(data);
        multi_done(data, result, TRUE);
        stream_error = TRUE;
      }
      break;

    case MSTATE_DO:
      if(data->set.connect_only) {
        /* keep connection open for application to use the socket */
        connkeep(data->conn, "CONNECT_ONLY");
        multistate(data, MSTATE_DONE);
        result = CURLE_OK;
        rc = CURLM_CALL_MULTI_PERFORM;
      }
      else {
        /* Perform the protocol's DO action */
        result = multi_do(data, &dophase_done);

        /* When multi_do() returns failure, data->conn might be NULL! */

        if(!result) {
          if(!dophase_done) {
#ifndef CURL_DISABLE_FTP
            /* some steps needed for wildcard matching */
            if(data->state.wildcardmatch) {
              struct WildcardData *wc = &data->wildcard;
              if(wc->state == CURLWC_DONE || wc->state == CURLWC_SKIP) {
                /* skip some states if it is important */
                multi_done(data, CURLE_OK, FALSE);

                /* if there's no connection left, skip the DONE state */
                multistate(data, data->conn ?
                           MSTATE_DONE : MSTATE_COMPLETED);
                rc = CURLM_CALL_MULTI_PERFORM;
                break;
              }
            }
#endif
            /* DO was not completed in one function call, we must continue
               DOING... */
            multistate(data, MSTATE_DOING);
            rc = CURLM_OK;
          }

          /* after DO, go DO_DONE... or DO_MORE */
          else if(data->conn->bits.do_more) {
            /* we're supposed to do more, but we need to sit down, relax
               and wait a little while first */
            multistate(data, MSTATE_DOING_MORE);
            rc = CURLM_OK;
          }
          else {
            /* we're done with the DO, now DID */
            multistate(data, MSTATE_DID);
            rc = CURLM_CALL_MULTI_PERFORM;
          }
        }
        else if((CURLE_SEND_ERROR == result) &&
                data->conn->bits.reuse) {
          /*
           * In this situation, a connection that we were trying to use
           * may have unexpectedly died.  If possible, send the connection
           * back to the CONNECT phase so we can try again.
           */
          char *newurl = NULL;
          followtype follow = FOLLOW_NONE;
          CURLcode drc;

          drc = Curl_retry_request(data, &newurl);
          if(drc) {
            /* a failure here pretty much implies an out of memory */
            result = drc;
            stream_error = TRUE;
          }

          Curl_posttransfer(data);
          drc = multi_done(data, result, FALSE);

          /* When set to retry the connection, we must to go back to
           * the CONNECT state */
          if(newurl) {
            if(!drc || (drc == CURLE_SEND_ERROR)) {
              follow = FOLLOW_RETRY;
              drc = Curl_follow(data, newurl, follow);
              if(!drc) {
                multistate(data, MSTATE_CONNECT);
                rc = CURLM_CALL_MULTI_PERFORM;
                result = CURLE_OK;
              }
              else {
                /* Follow failed */
                result = drc;
              }
            }
            else {
              /* done didn't return OK or SEND_ERROR */
              result = drc;
            }
          }
          else {
            /* Have error handler disconnect conn if we can't retry */
            stream_error = TRUE;
          }
          free(newurl);
        }
        else {
          /* failure detected */
          Curl_posttransfer(data);
          if(data->conn)
            multi_done(data, result, FALSE);
          stream_error = TRUE;
        }
      }
      break;

    case MSTATE_DOING:
      /* we continue DOING until the DO phase is complete */
      DEBUGASSERT(data->conn);
      result = protocol_doing(data, &dophase_done);
      if(!result) {
        if(dophase_done) {
          /* after DO, go DO_DONE or DO_MORE */
          multistate(data, data->conn->bits.do_more?
                     MSTATE_DOING_MORE : MSTATE_DID);
          rc = CURLM_CALL_MULTI_PERFORM;
        } /* dophase_done */
      }
      else {
        /* failure detected */
        Curl_posttransfer(data);
        multi_done(data, result, FALSE);
        stream_error = TRUE;
      }
      break;

    case MSTATE_DOING_MORE:
      /*
       * When we are connected, DOING MORE and then go DID
       */
      DEBUGASSERT(data->conn);
      result = multi_do_more(data, &control);

      if(!result) {
        if(control) {
          /* if positive, advance to DO_DONE
             if negative, go back to DOING */
          multistate(data, control == 1?
                     MSTATE_DID : MSTATE_DOING);
          rc = CURLM_CALL_MULTI_PERFORM;
        }
        else
          /* stay in DO_MORE */
          rc = CURLM_OK;
      }
      else {
        /* failure detected */
        Curl_posttransfer(data);
        multi_done(data, result, FALSE);
        stream_error = TRUE;
      }
      break;

    case MSTATE_DID:
      DEBUGASSERT(data->conn);
      if(data->conn->bits.multiplex)
        /* Check if we can move pending requests to send pipe */
        process_pending_handles(multi); /*  multiplexed */

      /* Only perform the transfer if there's a good socket to work with.
         Having both BAD is a signal to skip immediately to DONE */
      if((data->conn->sockfd != CURL_SOCKET_BAD) ||
         (data->conn->writesockfd != CURL_SOCKET_BAD))
        multistate(data, MSTATE_PERFORMING);
      else {
#ifndef CURL_DISABLE_FTP
        if(data->state.wildcardmatch &&
           ((data->conn->handler->flags & PROTOPT_WILDCARD) == 0)) {
          data->wildcard.state = CURLWC_DONE;
        }
#endif
        multistate(data, MSTATE_DONE);
      }
      rc = CURLM_CALL_MULTI_PERFORM;
      break;

    case MSTATE_RATELIMITING: /* limit-rate exceeded in either direction */
      DEBUGASSERT(data->conn);
      /* if both rates are within spec, resume transfer */
      if(Curl_pgrsUpdate(data))
        result = CURLE_ABORTED_BY_CALLBACK;
      else
        result = Curl_speedcheck(data, *nowp);

      if(result) {
        if(!(data->conn->handler->flags & PROTOPT_DUAL) &&
           result != CURLE_HTTP2_STREAM)
          streamclose(data->conn, "Transfer returned error");

        Curl_posttransfer(data);
        multi_done(data, result, TRUE);
      }
      else {
        send_timeout_ms = 0;
        if(data->set.max_send_speed)
          send_timeout_ms =
            Curl_pgrsLimitWaitTime(data->progress.uploaded,
                                   data->progress.ul_limit_size,
                                   data->set.max_send_speed,
                                   data->progress.ul_limit_start,
                                   *nowp);

        recv_timeout_ms = 0;
        if(data->set.max_recv_speed)
          recv_timeout_ms =
            Curl_pgrsLimitWaitTime(data->progress.downloaded,
                                   data->progress.dl_limit_size,
                                   data->set.max_recv_speed,
                                   data->progress.dl_limit_start,
                                   *nowp);

        if(!send_timeout_ms && !recv_timeout_ms) {
          multistate(data, MSTATE_PERFORMING);
          Curl_ratelimit(data, *nowp);
        }
        else if(send_timeout_ms >= recv_timeout_ms)
          Curl_expire(data, send_timeout_ms, EXPIRE_TOOFAST);
        else
          Curl_expire(data, recv_timeout_ms, EXPIRE_TOOFAST);
      }
      break;

    case MSTATE_PERFORMING:
    {
      char *newurl = NULL;
      bool retry = FALSE;
      bool comeback = FALSE;
      DEBUGASSERT(data->state.buffer);
      /* check if over send speed */
      send_timeout_ms = 0;
      if(data->set.max_send_speed)
        send_timeout_ms = Curl_pgrsLimitWaitTime(data->progress.uploaded,
                                                 data->progress.ul_limit_size,
                                                 data->set.max_send_speed,
                                                 data->progress.ul_limit_start,
                                                 *nowp);

      /* check if over recv speed */
      recv_timeout_ms = 0;
      if(data->set.max_recv_speed)
        recv_timeout_ms = Curl_pgrsLimitWaitTime(data->progress.downloaded,
                                                 data->progress.dl_limit_size,
                                                 data->set.max_recv_speed,
                                                 data->progress.dl_limit_start,
                                                 *nowp);

      if(send_timeout_ms || recv_timeout_ms) {
        Curl_ratelimit(data, *nowp);
        multistate(data, MSTATE_RATELIMITING);
        if(send_timeout_ms >= recv_timeout_ms)
          Curl_expire(data, send_timeout_ms, EXPIRE_TOOFAST);
        else
          Curl_expire(data, recv_timeout_ms, EXPIRE_TOOFAST);
        break;
      }

      /* read/write data if it is ready to do so */
      result = Curl_readwrite(data->conn, data, &done, &comeback);

      if(done || (result == CURLE_RECV_ERROR)) {
        /* If CURLE_RECV_ERROR happens early enough, we assume it was a race
         * condition and the server closed the re-used connection exactly when
         * we wanted to use it, so figure out if that is indeed the case.
         */
        CURLcode ret = Curl_retry_request(data, &newurl);
        if(!ret)
          retry = (newurl)?TRUE:FALSE;
        else if(!result)
          result = ret;

        if(retry) {
          /* if we are to retry, set the result to OK and consider the
             request as done */
          result = CURLE_OK;
          done = TRUE;
        }
      }
      else if((CURLE_HTTP2_STREAM == result) &&
              Curl_h2_http_1_1_error(data)) {
        CURLcode ret = Curl_retry_request(data, &newurl);

        if(!ret) {
          infof(data, "Downgrades to HTTP/1.1!");
          streamclose(data->conn, "Disconnect HTTP/2 for HTTP/1");
          data->state.httpwant = CURL_HTTP_VERSION_1_1;
          /* clear the error message bit too as we ignore the one we got */
          data->state.errorbuf = FALSE;
          if(!newurl)
            /* typically for HTTP_1_1_REQUIRED error on first flight */
            newurl = strdup(data->state.url);
          /* if we are to retry, set the result to OK and consider the request
             as done */
          retry = TRUE;
          result = CURLE_OK;
          done = TRUE;
        }
        else
          result = ret;
      }

      if(result) {
        /*
         * The transfer phase returned error, we mark the connection to get
         * closed to prevent being re-used. This is because we can't possibly
         * know if the connection is in a good shape or not now.  Unless it is
         * a protocol which uses two "channels" like FTP, as then the error
         * happened in the data connection.
         */

        if(!(data->conn->handler->flags & PROTOPT_DUAL) &&
           result != CURLE_HTTP2_STREAM)
          streamclose(data->conn, "Transfer returned error");

        Curl_posttransfer(data);
        multi_done(data, result, TRUE);
      }
      else if(done) {

        /* call this even if the readwrite function returned error */
        Curl_posttransfer(data);

        /* When we follow redirects or is set to retry the connection, we must
           to go back to the CONNECT state */
        if(data->req.newurl || retry) {
          followtype follow = FOLLOW_NONE;
          if(!retry) {
            /* if the URL is a follow-location and not just a retried request
               then figure out the URL here */
            free(newurl);
            newurl = data->req.newurl;
            data->req.newurl = NULL;
            follow = FOLLOW_REDIR;
          }
          else
            follow = FOLLOW_RETRY;
          (void)multi_done(data, CURLE_OK, FALSE);
          /* multi_done() might return CURLE_GOT_NOTHING */
          result = Curl_follow(data, newurl, follow);
          if(!result) {
            multistate(data, MSTATE_CONNECT);
            rc = CURLM_CALL_MULTI_PERFORM;
          }
          free(newurl);
        }
        else {
          /* after the transfer is done, go DONE */

          /* but first check to see if we got a location info even though we're
             not following redirects */
          if(data->req.location) {
            free(newurl);
            newurl = data->req.location;
            data->req.location = NULL;
            result = Curl_follow(data, newurl, FOLLOW_FAKE);
            free(newurl);
            if(result) {
              stream_error = TRUE;
              result = multi_done(data, result, TRUE);
            }
          }

          if(!result) {
            multistate(data, MSTATE_DONE);
            rc = CURLM_CALL_MULTI_PERFORM;
          }
        }
      }
      else if(comeback) {
        /* This avoids CURLM_CALL_MULTI_PERFORM so that a very fast transfer
           won't get stuck on this transfer at the expense of other concurrent
           transfers */
        Curl_expire(data, 0, EXPIRE_RUN_NOW);
        rc = CURLM_OK;
      }
      break;
    }

    case MSTATE_DONE:
      /* this state is highly transient, so run another loop after this */
      rc = CURLM_CALL_MULTI_PERFORM;

      if(data->conn) {
        CURLcode res;

        if(data->conn->bits.multiplex)
          /* Check if we can move pending requests to connection */
          process_pending_handles(multi); /* multiplexing */

        /* post-transfer command */
        res = multi_done(data, result, FALSE);

        /* allow a previously set error code take precedence */
        if(!result)
          result = res;
      }

#ifndef CURL_DISABLE_FTP
      if(data->state.wildcardmatch) {
        if(data->wildcard.state != CURLWC_DONE) {
          /* if a wildcard is set and we are not ending -> lets start again
             with MSTATE_INIT */
          multistate(data, MSTATE_INIT);
          break;
        }
      }
#endif
      /* after we have DONE what we're supposed to do, go COMPLETED, and
         it doesn't matter what the multi_done() returned! */
      multistate(data, MSTATE_COMPLETED);
      break;

    case MSTATE_COMPLETED:
      break;

    case MSTATE_MSGSENT:
      data->result = result;
      return CURLM_OK; /* do nothing */

    default:
      return CURLM_INTERNAL_ERROR;
    }

    if(data->conn &&
       data->mstate >= MSTATE_CONNECT &&
       data->mstate < MSTATE_DO &&
       rc != CURLM_CALL_MULTI_PERFORM &&
       !multi_ischanged(multi, false)) {
      /* We now handle stream timeouts if and only if this will be the last
       * loop iteration. We only check this on the last iteration to ensure
       * that if we know we have additional work to do immediately
       * (i.e. CURLM_CALL_MULTI_PERFORM == TRUE) then we should do that before
       * declaring the connection timed out as we may almost have a completed
       * connection. */
      multi_handle_timeout(data, nowp, &stream_error, &result, TRUE);
    }

    statemachine_end:

    if(data->mstate < MSTATE_COMPLETED) {
      if(result) {
        /*
         * If an error was returned, and we aren't in completed state now,
         * then we go to completed and consider this transfer aborted.
         */

        /* NOTE: no attempt to disconnect connections must be made
           in the case blocks above - cleanup happens only here */

        /* Check if we can move pending requests to send pipe */
        process_pending_handles(multi); /* connection */

        if(data->conn) {
          if(stream_error) {
            /* Don't attempt to send data over a connection that timed out */
            bool dead_connection = result == CURLE_OPERATION_TIMEDOUT;
            struct connectdata *conn = data->conn;

            /* This is where we make sure that the conn pointer is reset.
               We don't have to do this in every case block above where a
               failure is detected */
            Curl_detach_connnection(data);

            /* remove connection from cache */
            Curl_conncache_remove_conn(data, conn, TRUE);

            /* disconnect properly */
            Curl_disconnect(data, conn, dead_connection);
          }


// Source: multi.c
// Lines 1720-2494
