static int ossl_shutdown(struct Curl_easy *data,
                         struct connectdata *conn, int sockindex)
{
  int retval = 0;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  char buf[256]; /* We will use this for the OpenSSL error buffer, so it has
                    to be at least 256 bytes long. */
  unsigned long sslerror;
  ssize_t nread;
  int buffsize;
  int err;
  bool done = FALSE;
  struct ssl_backend_data *backend = connssl->backend;
  int loop = 10;

#ifndef CURL_DISABLE_FTP
  /* This has only been tested on the proftpd server, and the mod_tls code
     sends a close notify alert without waiting for a close notify alert in
     response. Thus we wait for a close notify alert from the server, but
     we do not send one. Let's hope other servers do the same... */

  if(data->set.ftp_ccc == CURLFTPSSL_CCC_ACTIVE)
      (void)SSL_shutdown(backend->handle);
#endif

  if(backend->handle) {
    buffsize = (int)sizeof(buf);
    while(!done && loop--) {
      int what = SOCKET_READABLE(conn->sock[sockindex],
                                 SSL_SHUTDOWN_TIMEOUT);
      if(what > 0) {
        ERR_clear_error();

        /* Something to read, let's do it and hope that it is the close
           notify alert from the server */
        nread = (ssize_t)SSL_read(backend->handle, buf, buffsize);
        err = SSL_get_error(backend->handle, (int)nread);

        switch(err) {
        case SSL_ERROR_NONE: /* this is not an error */
        case SSL_ERROR_ZERO_RETURN: /* no more data */
          /* This is the expected response. There was no data but only
             the close notify alert */
          done = TRUE;
          break;
        case SSL_ERROR_WANT_READ:
          /* there's data pending, re-invoke SSL_read() */
          infof(data, "SSL_ERROR_WANT_READ");
          break;
        case SSL_ERROR_WANT_WRITE:
          /* SSL wants a write. Really odd. Let's bail out. */
          infof(data, "SSL_ERROR_WANT_WRITE");
          done = TRUE;
          break;
        default:
          /* openssl/ssl.h says "look at error stack/return value/errno" */
          sslerror = ERR_get_error();
          failf(data, OSSL_PACKAGE " SSL_read on shutdown: %s, errno %d",
                (sslerror ?
                 ossl_strerror(sslerror, buf, sizeof(buf)) :
                 SSL_ERROR_to_str(err)),
                SOCKERRNO);
          done = TRUE;
          break;
        }
      }
      else if(0 == what) {
        /* timeout */
        failf(data, "SSL shutdown timeout");
        done = TRUE;
      }
      else {
        /* anything that gets here is fatally bad */
        failf(data, "select/poll on SSL socket, errno: %d", SOCKERRNO);
        retval = -1;
        done = TRUE;
      }
    } /* while()-loop for the select() */

    if(data->set.verbose) {
#ifdef HAVE_SSL_GET_SHUTDOWN
      switch(SSL_get_shutdown(backend->handle)) {
      case SSL_SENT_SHUTDOWN:
        infof(data, "SSL_get_shutdown() returned SSL_SENT_SHUTDOWN");
        break;
      case SSL_RECEIVED_SHUTDOWN:
        infof(data, "SSL_get_shutdown() returned SSL_RECEIVED_SHUTDOWN");
        break;
      case SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN:
        infof(data, "SSL_get_shutdown() returned SSL_SENT_SHUTDOWN|"
              "SSL_RECEIVED__SHUTDOWN");
        break;
      }
#endif
    }

    SSL_free(backend->handle);
    backend->handle = NULL;
  }
  return retval;
}


// Source: openssl.c
// Lines 1437-1537
