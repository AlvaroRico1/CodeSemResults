// Source: curl/lib/ftp.c
// Lines 368-370
static CURLcode ReceivedServerConnect(struct Curl_easy *data, bool *received)
{
  struct connectdata *conn = data->conn;
  curl_socket_t ctrl_sock = conn->sock[FIRSTSOCKET];
  curl_socket_t data_sock = conn->sock[SECONDARYSOCKET];
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  struct pingpong *pp = &ftpc->pp;
  int result;
  timediff_t timeout_ms;
  ssize_t nread;
  int ftpcode;

  *received = FALSE;

  timeout_ms = ftp_timeleft_accept(data);
  infof(data, "Checking for server connect");
  if(timeout_ms < 0) {
    /* if a timeout was already reached, bail out */
    failf(data, "Accept timeout occurred while waiting server connect");
    return CURLE_FTP_ACCEPT_TIMEOUT;
  }

  /* First check whether there is a cached response from server */
  if(pp->cache_size && pp->cache && pp->cache[0] > '3') {
    /* Data connection could not be established, let's return */
    infof(data, "There is negative response in cache while serv connect");
    (void)Curl_GetFTPResponse(data, &nread, &ftpcode);
    return CURLE_FTP_ACCEPT_FAILED;
  }

  result = Curl_socket_check(ctrl_sock, data_sock, CURL_SOCKET_BAD, 0);

  /* see if the connection request is already here */
  switch(result) {
  case -1: /* error */
    /* let's die here */
    failf(data, "Error while waiting for server connect");
    return CURLE_FTP_ACCEPT_FAILED;
  case 0:  /* Server connect is not received yet */
    break; /* loop */
  default:

    if(result & CURL_CSELECT_IN2) {
      infof(data, "Ready to accept data connection from server");
      *received = TRUE;
    }
    else if(result & CURL_CSELECT_IN) {
      infof(data, "Ctrl conn has data while waiting for data conn");
      (void)Curl_GetFTPResponse(data, &nread, &ftpcode);

      if(ftpcode/100 > 3)
        return CURLE_FTP_ACCEPT_FAILED;

      return CURLE_WEIRD_SERVER_REPLY;
    }

    break;
  } /* switch() */

  return CURLE_OK;
}
