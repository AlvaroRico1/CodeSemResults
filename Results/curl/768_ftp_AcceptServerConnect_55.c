static CURLcode AcceptServerConnect(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  curl_socket_t sock = conn->sock[SECONDARYSOCKET];
  curl_socket_t s = CURL_SOCKET_BAD;
#ifdef ENABLE_IPV6
  struct Curl_sockaddr_storage add;
#else
  struct sockaddr_in add;
#endif
  curl_socklen_t size = (curl_socklen_t) sizeof(add);

  if(0 == getsockname(sock, (struct sockaddr *) &add, &size)) {
    size = sizeof(add);

    s = accept(sock, (struct sockaddr *) &add, &size);
  }
  Curl_closesocket(data, conn, sock); /* close the first socket */

  if(CURL_SOCKET_BAD == s) {
    failf(data, "Error accept()ing server connect");
    return CURLE_FTP_PORT_FAILED;
  }
  infof(data, "Connection accepted from server");
  /* when this happens within the DO state it is important that we mark us as
     not needing DO_MORE anymore */
  conn->bits.do_more = FALSE;

  conn->sock[SECONDARYSOCKET] = s;
  (void)curlx_nonblock(s, TRUE); /* enable non-blocking */
  conn->bits.sock_accepted = TRUE;

  if(data->set.fsockopt) {
    int error = 0;

    /* activate callback for setting socket options */
    Curl_set_in_callback(data, true);
    error = data->set.fsockopt(data->set.sockopt_client,
                               s,
                               CURLSOCKTYPE_ACCEPT);
    Curl_set_in_callback(data, false);

    if(error) {
      close_secondarysocket(data, conn);
      return CURLE_ABORTED_BY_CALLBACK;
    }
  }

  return CURLE_OK;

}


// Source: ftp.c
// Lines 269-319
