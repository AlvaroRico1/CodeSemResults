// Source: curl/lib/sendf.c
// Lines 339-342
ssize_t Curl_send_plain(struct Curl_easy *data, int num,
                        const void *mem, size_t len, CURLcode *code)
{
  struct connectdata *conn;
  curl_socket_t sockfd;
  ssize_t bytes_written;

  DEBUGASSERT(data);
  DEBUGASSERT(data->conn);
  conn = data->conn;
  sockfd = conn->sock[num];
  /* WinSock will destroy unread received data if send() is
     failed.
     To avoid lossage of received data, recv() must be
     performed before every send() if any incoming data is
     available. */
  if(pre_receive_plain(data, conn, num)) {
    *code = CURLE_OUT_OF_MEMORY;
    return -1;
  }

#if defined(MSG_FASTOPEN) && !defined(TCP_FASTOPEN_CONNECT) /* Linux */
  if(conn->bits.tcp_fastopen) {
    bytes_written = sendto(sockfd, mem, len, MSG_FASTOPEN,
                           conn->ip_addr->ai_addr, conn->ip_addr->ai_addrlen);
    conn->bits.tcp_fastopen = FALSE;
  }
  else
#endif
    bytes_written = swrite(sockfd, mem, len);

  *code = CURLE_OK;
  if(-1 == bytes_written) {
    int err = SOCKERRNO;

    if(
#ifdef WSAEWOULDBLOCK
      /* This is how Windows does it */
      (WSAEWOULDBLOCK == err)
#else
      /* errno may be EWOULDBLOCK or on some systems EAGAIN when it returned
         due to its inability to send off data without blocking. We therefore
         treat both error codes the same here */
      (EWOULDBLOCK == err) || (EAGAIN == err) || (EINTR == err) ||
      (EINPROGRESS == err)
#endif
      ) {
      /* this is just a case of EWOULDBLOCK */
      bytes_written = 0;
      *code = CURLE_AGAIN;
    }
    else {
      char buffer[STRERROR_LEN];
      failf(data, "Send failure: %s",
            Curl_strerror(err, buffer, sizeof(buffer)));
      data->state.os_errno = err;
      *code = CURLE_SEND_ERROR;
    }
  }
  return bytes_written;
}
