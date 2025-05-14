// Source: curl/lib/vtls/openssl.c
// Lines 4286-4287
static ssize_t ossl_recv(struct Curl_easy *data,   /* transfer */
                         int num,                  /* socketindex */
                         char *buf,                /* store read data here */
                         size_t buffersize,        /* max amount to read */
                         CURLcode *curlcode)
{
  char error_buffer[256];
  unsigned long sslerror;
  ssize_t nread;
  int buffsize;
  struct connectdata *conn = data->conn;
  struct ssl_connect_data *connssl = &conn->ssl[num];
  struct ssl_backend_data *backend = connssl->backend;

  ERR_clear_error();

  buffsize = (buffersize > (size_t)INT_MAX) ? INT_MAX : (int)buffersize;
  set_logger(conn, data);
  nread = (ssize_t)SSL_read(backend->handle, buf, buffsize);
  if(nread <= 0) {
    /* failed SSL_read */
    int err = SSL_get_error(backend->handle, (int)nread);

    switch(err) {
    case SSL_ERROR_NONE: /* this is not an error */
      break;
    case SSL_ERROR_ZERO_RETURN: /* no more data */
      /* close_notify alert */
      if(num == FIRSTSOCKET)
        /* mark the connection for close if it is indeed the control
           connection */
        connclose(conn, "TLS close_notify");
      break;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      /* there's data pending, re-invoke SSL_read() */
      *curlcode = CURLE_AGAIN;
      return -1;
    default:
      /* openssl/ssl.h for SSL_ERROR_SYSCALL says "look at error stack/return
         value/errno" */
      /* https://www.openssl.org/docs/crypto/ERR_get_error.html */
      sslerror = ERR_get_error();
      if((nread < 0) || sslerror) {
        /* If the return code was negative or there actually is an error in the
           queue */
        int sockerr = SOCKERRNO;
        if(sslerror)
          ossl_strerror(sslerror, error_buffer, sizeof(error_buffer));
        else if(sockerr && err == SSL_ERROR_SYSCALL)
          Curl_strerror(sockerr, error_buffer, sizeof(error_buffer));
        else {
          strncpy(error_buffer, SSL_ERROR_to_str(err), sizeof(error_buffer));
          error_buffer[sizeof(error_buffer) - 1] = '\0';
        }
        failf(data, OSSL_PACKAGE " SSL_read: %s, errno %d",
              error_buffer, sockerr);
        *curlcode = CURLE_RECV_ERROR;
        return -1;
      }
      /* For debug builds be a little stricter and error on any
         SSL_ERROR_SYSCALL. For example a server may have closed the connection
         abruptly without a close_notify alert. For compatibility with older
         peers we don't do this by default. #4624

         We can use this to gauge how many users may be affected, and
         if it goes ok eventually transition to allow in dev and release with
         the newest OpenSSL: #if (OPENSSL_VERSION_NUMBER >= 0x10101000L) */
#ifdef DEBUGBUILD
      if(err == SSL_ERROR_SYSCALL) {
        int sockerr = SOCKERRNO;
        if(sockerr)
          Curl_strerror(sockerr, error_buffer, sizeof(error_buffer));
        else {
          msnprintf(error_buffer, sizeof(error_buffer),
                    "Connection closed abruptly");
        }
        failf(data, OSSL_PACKAGE " SSL_read: %s, errno %d"
              " (Fatal because this is a curl debug build)",
              error_buffer, sockerr);
        *curlcode = CURLE_RECV_ERROR;
        return -1;
      }
#endif
    }
  }
  return nread;
}
