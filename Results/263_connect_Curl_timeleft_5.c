// Source: curl/lib/connect.c
// Lines 185-192
timediff_t Curl_timeleft(struct Curl_easy *data,
                         struct curltime *nowp,
                         bool duringconnect)
{
  unsigned int timeout_set = 0;
  timediff_t connect_timeout_ms = 0;
  timediff_t maxtime_timeout_ms = 0;
  timediff_t timeout_ms = 0;
  struct curltime now;

  /* The duration of a connect and the total transfer are calculated from two
     different time-stamps. It can end up with the total timeout being reached
     before the connect timeout expires and we must acknowledge whichever
     timeout that is reached first. The total timeout is set per entire
     operation, while the connect timeout is set per connect. */

  if(data->set.timeout > 0) {
    timeout_set = TIMEOUT_MAXTIME;
    maxtime_timeout_ms = data->set.timeout;
  }
  if(duringconnect) {
    timeout_set |= TIMEOUT_CONNECT;
    connect_timeout_ms = (data->set.connecttimeout > 0) ?
      data->set.connecttimeout : DEFAULT_CONNECT_TIMEOUT;
  }
  if(!timeout_set)
    /* no timeout  */
    return 0;

  if(!nowp) {
    now = Curl_now();
    nowp = &now;
  }

  if(timeout_set & TIMEOUT_MAXTIME) {
    maxtime_timeout_ms -= Curl_timediff(*nowp, data->progress.t_startop);
    timeout_ms = maxtime_timeout_ms;
  }

  if(timeout_set & TIMEOUT_CONNECT) {
    connect_timeout_ms -= Curl_timediff(*nowp, data->progress.t_startsingle);

    if(!(timeout_set & TIMEOUT_MAXTIME) ||
       (connect_timeout_ms < maxtime_timeout_ms))
      timeout_ms = connect_timeout_ms;
  }

  if(!timeout_ms)
    /* avoid returning 0 as that means no timeout! */
    return -1;

  return timeout_ms;
}
