// Source: curl/lib/vtls/openssl.c
// Lines 2119-2120
static void ossl_trace(int direction, int ssl_ver, int content_type,
                       const void *buf, size_t len, SSL *ssl,
                       void *userp)
{
  char unknown[32];
  const char *verstr = NULL;
  struct connectdata *conn = userp;
  struct ssl_connect_data *connssl = &conn->ssl[0];
  struct ssl_backend_data *backend = connssl->backend;
  struct Curl_easy *data = backend->logger;

  if(!conn || !data || !data->set.fdebug ||
     (direction != 0 && direction != 1))
    return;

  switch(ssl_ver) {
#ifdef SSL2_VERSION /* removed in recent versions */
  case SSL2_VERSION:
    verstr = "SSLv2";
    break;
#endif
#ifdef SSL3_VERSION
  case SSL3_VERSION:
    verstr = "SSLv3";
    break;
#endif
  case TLS1_VERSION:
    verstr = "TLSv1.0";
    break;
#ifdef TLS1_1_VERSION
  case TLS1_1_VERSION:
    verstr = "TLSv1.1";
    break;
#endif
#ifdef TLS1_2_VERSION
  case TLS1_2_VERSION:
    verstr = "TLSv1.2";
    break;
#endif
#ifdef TLS1_3_VERSION
  case TLS1_3_VERSION:
    verstr = "TLSv1.3";
    break;
#endif
  case 0:
    break;
  default:
    msnprintf(unknown, sizeof(unknown), "(%x)", ssl_ver);
    verstr = unknown;
    break;
  }

  /* Log progress for interesting records only (like Handshake or Alert), skip
   * all raw record headers (content_type == SSL3_RT_HEADER or ssl_ver == 0).
   * For TLS 1.3, skip notification of the decrypted inner Content-Type.
   */
  if(ssl_ver
#ifdef SSL3_RT_INNER_CONTENT_TYPE
     && content_type != SSL3_RT_INNER_CONTENT_TYPE
#endif
    ) {
    const char *msg_name, *tls_rt_name;
    char ssl_buf[1024];
    int msg_type, txt_len;

    /* the info given when the version is zero is not that useful for us */

    ssl_ver >>= 8; /* check the upper 8 bits only below */

    /* SSLv2 doesn't seem to have TLS record-type headers, so OpenSSL
     * always pass-up content-type as 0. But the interesting message-type
     * is at 'buf[0]'.
     */
    if(ssl_ver == SSL3_VERSION_MAJOR && content_type)
      tls_rt_name = tls_rt_type(content_type);
    else
      tls_rt_name = "";

    if(content_type == SSL3_RT_CHANGE_CIPHER_SPEC) {
      msg_type = *(char *)buf;
      msg_name = "Change cipher spec";
    }
    else if(content_type == SSL3_RT_ALERT) {
      msg_type = (((char *)buf)[0] << 8) + ((char *)buf)[1];
      msg_name = SSL_alert_desc_string_long(msg_type);
    }
    else {
      msg_type = *(char *)buf;
      msg_name = ssl_msg_type(ssl_ver, msg_type);
    }

    txt_len = msnprintf(ssl_buf, sizeof(ssl_buf), "%s (%s), %s, %s (%d):\n",
                        verstr, direction?"OUT":"IN",
                        tls_rt_name, msg_name, msg_type);
    if(0 <= txt_len && (unsigned)txt_len < sizeof(ssl_buf)) {
      Curl_debug(data, CURLINFO_TEXT, ssl_buf, (size_t)txt_len);
    }
  }

  Curl_debug(data, (direction == 1) ? CURLINFO_SSL_DATA_OUT :
             CURLINFO_SSL_DATA_IN, (char *)buf, len);
  (void) ssl;
}
