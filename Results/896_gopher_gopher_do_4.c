static CURLcode gopher_do(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  char *gopherpath;
  char *path = data->state.up.path;
  char *query = data->state.up.query;
  char *sel = NULL;
  char *sel_org = NULL;
  timediff_t timeout_ms;
  ssize_t amount, k;
  size_t len;
  int what;

  *done = TRUE; /* unconditionally */

  /* path is guaranteed non-NULL */
  DEBUGASSERT(path);

  if(query)
    gopherpath = aprintf("%s?%s", path, query);
  else
    gopherpath = strdup(path);

  if(!gopherpath)
    return CURLE_OUT_OF_MEMORY;

  /* Create selector. Degenerate cases: / and /1 => convert to "" */
  if(strlen(gopherpath) <= 2) {
    sel = (char *)"";
    len = strlen(sel);
    free(gopherpath);
  }
  else {
    char *newp;

    /* Otherwise, drop / and the first character (i.e., item type) ... */
    newp = gopherpath;
    newp += 2;

    /* ... and finally unescape */
    result = Curl_urldecode(data, newp, 0, &sel, &len, REJECT_ZERO);
    free(gopherpath);
    if(result)
      return result;
    sel_org = sel;
  }

  k = curlx_uztosz(len);

  for(;;) {
    /* Break out of the loop if the selector is empty because OpenSSL and/or
       LibreSSL fail with errno 0 if this is the case. */
    if(strlen(sel) < 1)
      break;

    result = Curl_write(data, sockfd, sel, k, &amount);
    if(!result) { /* Which may not have written it all! */
      result = Curl_client_write(data, CLIENTWRITE_HEADER, sel, amount);
      if(result)
        break;

      k -= amount;
      sel += amount;
      if(k < 1)
        break; /* but it did write it all */
    }
    else
      break;

    timeout_ms = Curl_timeleft(data, NULL, FALSE);
    if(timeout_ms < 0) {
      result = CURLE_OPERATION_TIMEDOUT;
      break;
    }
    if(!timeout_ms)
      timeout_ms = TIMEDIFF_T_MAX;

    /* Don't busyloop. The entire loop thing is a work-around as it causes a
       BLOCKING behavior which is a NO-NO. This function should rather be
       split up in a do and a doing piece where the pieces that aren't
       possible to send now will be sent in the doing function repeatedly
       until the entire request is sent.
    */
    what = SOCKET_WRITABLE(sockfd, timeout_ms);
    if(what < 0) {
      result = CURLE_SEND_ERROR;
      break;
    }
    else if(!what) {
      result = CURLE_OPERATION_TIMEDOUT;
      break;
    }
  }

  free(sel_org);

  if(!result)
    result = Curl_write(data, sockfd, "\r\n", 2, &amount);
  if(result) {
    failf(data, "Failed sending Gopher request");
    return result;
  }
  result = Curl_client_write(data, CLIENTWRITE_HEADER, (char *)"\r\n", 2);
  if(result)
    return result;

  Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
  return CURLE_OK;
}


// Source: gopher.c
// Lines 126-236
