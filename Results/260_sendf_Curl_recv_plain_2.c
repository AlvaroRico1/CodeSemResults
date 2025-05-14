// Source: curl/lib/sendf.c
// Lines 423-426
ssize_t Curl_recv_plain(struct Curl_easy *data, int num, char *buf,
                        size_t len, CURLcode *code)
{
  struct connectdata *conn;
  curl_socket_t sockfd;
  ssize_t nread;
  DEBUGASSERT(data);
  DEBUGASSERT(data->conn);
  conn = data->conn;
  sockfd = conn->sock[num];
  /* Check and return data that already received and storied in internal
     intermediate buffer */
  nread = get_pre_recved(conn, num, buf, len);
  if(nread > 0) {
    *code = CURLE_OK;
    return nread;
  }

  nread = sread(sockfd, buf, len);

  *code = CURLE_OK;
  if(-1 == nread) {
    int err = SOCKERRNO;

    if(
#ifdef WSAEWOULDBLOCK
      /* This is how Windows does it */
      (WSAEWOULDBLOCK == err)
#else
      /* errno may be EWOULDBLOCK or on some systems EAGAIN when it returned
         due to its inability to send off data without blocking. We therefore
         treat both error codes the same here */
      (EWOULDBLOCK == err) || (EAGAIN == err) || (EINTR == err)
#endif
      ) {
      /* this is just a case of EWOULDBLOCK */
      *code = CURLE_AGAIN;
    }
    else {
      char buffer[STRERROR_LEN];
      failf(data, "Recv failure: %s",
            Curl_strerror(err, buffer, sizeof(buffer)));
      data->state.os_errno = err;
      *code = CURLE_RECV_ERROR;
    }
  }
  return nread;
}
