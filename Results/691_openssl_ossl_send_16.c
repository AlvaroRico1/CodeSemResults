static ssize_t ossl_send(struct Curl_easy *data,
                         int sockindex,
                         const void *mem,
                         size_t len,
                         CURLcode *curlcode)
{
  /* SSL_write() is said to return 'int' while write() and send() returns
     'size_t' */
  int err;
  char error_buffer[256];
  unsigned long sslerror;
  int memlen;
  int rc;
  struct connectdata *conn = data->conn;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  struct ssl_backend_data *backend = connssl->backend;

  ERR_clear_error();

  memlen = (len > (size_t)INT_MAX) ? INT_MAX : (int)len;
  set_logger(conn, data);
  rc = SSL_write(backend->handle, mem, memlen);

  if(rc <= 0) {
    err = SSL_get_error(backend->handle, rc);

    switch(err) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      /* The operation did not complete; the same TLS/SSL I/O function
         should be called again later. This is basically an EWOULDBLOCK
         equivalent. */
      *curlcode = CURLE_AGAIN;
      return -1;
    case SSL_ERROR_SYSCALL:
      {
        int sockerr = SOCKERRNO;
        sslerror = ERR_get_error();
        if(sslerror)
          ossl_strerror(sslerror, error_buffer, sizeof(error_buffer));
        else if(sockerr)
          Curl_strerror(sockerr, error_buffer, sizeof(error_buffer));
        else {
          strncpy(error_buffer, SSL_ERROR_to_str(err), sizeof(error_buffer));
          error_buffer[sizeof(error_buffer) - 1] = '\0';
        }
        failf(data, OSSL_PACKAGE " SSL_write: %s, errno %d",
              error_buffer, sockerr);
        *curlcode = CURLE_SEND_ERROR;
        return -1;
      }
    case SSL_ERROR_SSL:
      /*  A failure in the SSL library occurred, usually a protocol error.
          The OpenSSL error queue contains more information on the error. */
      sslerror = ERR_get_error();
      if(ERR_GET_LIB(sslerror) == ERR_LIB_SSL &&
         ERR_GET_REASON(sslerror) == SSL_R_BIO_NOT_SET &&
         conn->ssl[sockindex].state == ssl_connection_complete
#ifndef CURL_DISABLE_PROXY
         && conn->proxy_ssl[sockindex].state == ssl_connection_complete
#endif
        ) {
        char ver[120];
        (void)ossl_version(ver, sizeof(ver));
        failf(data, "Error: %s does not support double SSL tunneling.", ver);
      }
      else
        failf(data, "SSL_write() error: %s",
              ossl_strerror(sslerror, error_buffer, sizeof(error_buffer)));
      *curlcode = CURLE_SEND_ERROR;
      return -1;
    }
    /* a true error */
    failf(data, OSSL_PACKAGE " SSL_write: %s, errno %d",
          SSL_ERROR_to_str(err), SOCKERRNO);
    *curlcode = CURLE_SEND_ERROR;
    return -1;
  }
  *curlcode = CURLE_OK;
  return (ssize_t)rc; /* number of bytes */
}


// Source: openssl.c
// Lines 4193-4273
