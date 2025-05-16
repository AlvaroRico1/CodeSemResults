CURLcode Curl_pp_statemach(struct Curl_easy *data,
                           struct pingpong *pp, bool block,
                           bool disconnecting)
{
  struct connectdata *conn = data->conn;
  curl_socket_t sock = conn->sock[FIRSTSOCKET];
  int rc;
  timediff_t interval_ms;
  timediff_t timeout_ms = Curl_pp_state_timeout(data, pp, disconnecting);
  CURLcode result = CURLE_OK;

  if(timeout_ms <= 0) {
    failf(data, "server response timeout");
    return CURLE_OPERATION_TIMEDOUT; /* already too little time */
  }

  if(block) {
    interval_ms = 1000;  /* use 1 second timeout intervals */
    if(timeout_ms < interval_ms)
      interval_ms = timeout_ms;
  }
  else
    interval_ms = 0; /* immediate */

  if(Curl_ssl_data_pending(conn, FIRSTSOCKET))
    rc = 1;
  else if(Curl_pp_moredata(pp))
    /* We are receiving and there is data in the cache so just read it */
    rc = 1;
  else if(!pp->sendleft && Curl_ssl_data_pending(conn, FIRSTSOCKET))
    /* We are receiving and there is data ready in the SSL library */
    rc = 1;
  else
    rc = Curl_socket_check(pp->sendleft?CURL_SOCKET_BAD:sock, /* reading */
                           CURL_SOCKET_BAD,
                           pp->sendleft?sock:CURL_SOCKET_BAD, /* writing */
                           interval_ms);

  if(block) {
    /* if we didn't wait, we don't have to spend time on this now */
    if(Curl_pgrsUpdate(data))
      result = CURLE_ABORTED_BY_CALLBACK;
    else
      result = Curl_speedcheck(data, Curl_now());

    if(result)
      return result;
  }

  if(rc == -1) {
    failf(data, "select/poll error");
    result = CURLE_OUT_OF_MEMORY;
  }
  else if(rc)
    result = pp->statemachine(data, data->conn);

  return result;
}


// Source: pingpong.c
// Lines 80-137
