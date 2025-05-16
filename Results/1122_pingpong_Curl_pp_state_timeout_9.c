timediff_t Curl_pp_state_timeout(struct Curl_easy *data,
                                 struct pingpong *pp, bool disconnecting)
{
  struct connectdata *conn = data->conn;
  timediff_t timeout_ms; /* in milliseconds */
  timediff_t response_time = (data->set.server_response_timeout)?
    data->set.server_response_timeout: pp->response_time;

  /* if CURLOPT_SERVER_RESPONSE_TIMEOUT is set, use that to determine
     remaining time, or use pp->response because SERVER_RESPONSE_TIMEOUT is
     supposed to govern the response for any given server response, not for
     the time from connect to the given server response. */

  /* Without a requested timeout, we only wait 'response_time' seconds for the
     full response to arrive before we bail out */
  timeout_ms = response_time -
    Curl_timediff(Curl_now(), pp->response); /* spent time */

  if(data->set.timeout && !disconnecting) {
    /* if timeout is requested, find out how much remaining time we have */
    timediff_t timeout2_ms = data->set.timeout - /* timeout time */
      Curl_timediff(Curl_now(), conn->now); /* spent time */

    /* pick the lowest number */
    timeout_ms = CURLMIN(timeout_ms, timeout2_ms);
  }

  return timeout_ms;
}


// Source: pingpong.c
// Lines 47-75
