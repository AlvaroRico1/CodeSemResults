static CURLcode ossl_connect_step2(struct Curl_easy *data,
                                   struct connectdata *conn, int sockindex)
{
  int err;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  struct ssl_backend_data *backend = connssl->backend;
  DEBUGASSERT(ssl_connect_2 == connssl->connecting_state
              || ssl_connect_2_reading == connssl->connecting_state
              || ssl_connect_2_writing == connssl->connecting_state);

  ERR_clear_error();

  err = SSL_connect(backend->handle);
#ifndef HAVE_KEYLOG_CALLBACK
  if(Curl_tls_keylog_enabled()) {
    /* If key logging is enabled, wait for the handshake to complete and then
     * proceed with logging secrets (for TLS 1.2 or older).
     */
    ossl_log_tls12_secret(backend->handle, &backend->keylog_done);
  }
#endif

  /* 1  is fine
     0  is "not successful but was shut down controlled"
     <0 is "handshake was not successful, because a fatal error occurred" */
  if(1 != err) {
    int detail = SSL_get_error(backend->handle, err);

    if(SSL_ERROR_WANT_READ == detail) {
      connssl->connecting_state = ssl_connect_2_reading;
      return CURLE_OK;
    }
    if(SSL_ERROR_WANT_WRITE == detail) {
      connssl->connecting_state = ssl_connect_2_writing;
      return CURLE_OK;
    }
#ifdef SSL_ERROR_WANT_ASYNC
    if(SSL_ERROR_WANT_ASYNC == detail) {
      connssl->connecting_state = ssl_connect_2;
      return CURLE_OK;
    }
#endif
    else {
      /* untreated error */
      unsigned long errdetail;
      char error_buffer[256]="";
      CURLcode result;
      long lerr;
      int lib;
      int reason;

      /* the connection failed, we're not waiting for anything else. */
      connssl->connecting_state = ssl_connect_2;

      /* Get the earliest error code from the thread's error queue and remove
         the entry. */
      errdetail = ERR_get_error();

      /* Extract which lib and reason */
      lib = ERR_GET_LIB(errdetail);
      reason = ERR_GET_REASON(errdetail);

      if((lib == ERR_LIB_SSL) &&
         ((reason == SSL_R_CERTIFICATE_VERIFY_FAILED) ||
          (reason == SSL_R_SSLV3_ALERT_CERTIFICATE_EXPIRED))) {
        result = CURLE_PEER_FAILED_VERIFICATION;

        lerr = SSL_get_verify_result(backend->handle);
        if(lerr != X509_V_OK) {
          SSL_SET_OPTION_LVALUE(certverifyresult) = lerr;
          msnprintf(error_buffer, sizeof(error_buffer),
                    "SSL certificate problem: %s",
                    X509_verify_cert_error_string(lerr));
        }
        else
          /* strcpy() is fine here as long as the string fits within
             error_buffer */
          strcpy(error_buffer, "SSL certificate verification failed");
      }
#if (OPENSSL_VERSION_NUMBER >= 0x10101000L && \
    !defined(LIBRESSL_VERSION_NUMBER) && \
    !defined(OPENSSL_IS_BORINGSSL))
      /* SSL_R_TLSV13_ALERT_CERTIFICATE_REQUIRED is only available on
         OpenSSL version above v1.1.1, not LibreSSL nor BoringSSL */
      else if((lib == ERR_LIB_SSL) &&
              (reason == SSL_R_TLSV13_ALERT_CERTIFICATE_REQUIRED)) {
          /* If client certificate is required, communicate the
             error to client */
          result = CURLE_SSL_CLIENTCERT;
          ossl_strerror(errdetail, error_buffer, sizeof(error_buffer));
      }
#endif
      else {
        result = CURLE_SSL_CONNECT_ERROR;
        ossl_strerror(errdetail, error_buffer, sizeof(error_buffer));
      }

      /* detail is already set to the SSL error above */

      /* If we e.g. use SSLv2 request-method and the server doesn't like us
       * (RST connection, etc.), OpenSSL gives no explanation whatsoever and
       * the SO_ERROR is also lost.
       */
      if(CURLE_SSL_CONNECT_ERROR == result && errdetail == 0) {
        const char * const hostname = SSL_HOST_NAME();
        const long int port = SSL_HOST_PORT();
        char extramsg[80]="";
        int sockerr = SOCKERRNO;
        if(sockerr && detail == SSL_ERROR_SYSCALL)
          Curl_strerror(sockerr, extramsg, sizeof(extramsg));
        failf(data, OSSL_PACKAGE " SSL_connect: %s in connection to %s:%ld ",
              extramsg[0] ? extramsg : SSL_ERROR_to_str(detail),
              hostname, port);
        return result;
      }

      /* Could be a CERT problem */
      failf(data, "%s", error_buffer);

      return result;
    }
  }
  else {
    /* we connected fine, we're not waiting for anything else. */
    connssl->connecting_state = ssl_connect_3;

    /* Informational message */
    infof(data, "SSL connection using %s / %s",
          SSL_get_version(backend->handle),
          SSL_get_cipher(backend->handle));

#ifdef HAS_ALPN
    /* Sets data and len to negotiated protocol, len is 0 if no protocol was
     * negotiated
     */
    if(conn->bits.tls_enable_alpn) {
      const unsigned char *neg_protocol;
      unsigned int len;
      SSL_get0_alpn_selected(backend->handle, &neg_protocol, &len);
      if(len) {
        infof(data, "ALPN, server accepted to use %.*s", len, neg_protocol);

#ifdef USE_HTTP2
        if(len == ALPN_H2_LENGTH &&
           !memcmp(ALPN_H2, neg_protocol, len)) {
          conn->negnpn = CURL_HTTP_VERSION_2;
        }
        else
#endif
        if(len == ALPN_HTTP_1_1_LENGTH &&
           !memcmp(ALPN_HTTP_1_1, neg_protocol, ALPN_HTTP_1_1_LENGTH)) {
          conn->negnpn = CURL_HTTP_VERSION_1_1;
        }
      }
      else
        infof(data, "ALPN, server did not agree to a protocol");

      Curl_multiuse_state(data, conn->negnpn == CURL_HTTP_VERSION_2 ?
                          BUNDLE_MULTIPLEX : BUNDLE_NO_MULTIUSE);
    }
#endif

    return CURLE_OK;
  }
}


// Source: openssl.c
// Lines 3284-3448
