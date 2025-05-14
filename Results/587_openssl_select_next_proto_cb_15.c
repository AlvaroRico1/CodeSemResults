// Source: curl/lib/vtls/openssl.c
// Lines 2267-2269
select_next_proto_cb(SSL *ssl,
                     unsigned char **out, unsigned char *outlen,
                     const unsigned char *in, unsigned int inlen,
                     void *arg)
{
  struct Curl_easy *data = (struct Curl_easy *)arg;
  struct connectdata *conn = data->conn;
  (void)ssl;

#ifdef USE_HTTP2
  if(data->state.httpwant >= CURL_HTTP_VERSION_2 &&
     !select_next_protocol(out, outlen, in, inlen, ALPN_H2, ALPN_H2_LENGTH)) {
    infof(data, "NPN, negotiated HTTP2 (%s)", ALPN_H2);
    conn->negnpn = CURL_HTTP_VERSION_2;
    return SSL_TLSEXT_ERR_OK;
  }
#endif

  if(!select_next_protocol(out, outlen, in, inlen, ALPN_HTTP_1_1,
                           ALPN_HTTP_1_1_LENGTH)) {
    infof(data, "NPN, negotiated HTTP1.1");
    conn->negnpn = CURL_HTTP_VERSION_1_1;
    return SSL_TLSEXT_ERR_OK;
  }

  infof(data, "NPN, no overlap, use HTTP1.1");
  *out = (unsigned char *)ALPN_HTTP_1_1;
  *outlen = ALPN_HTTP_1_1_LENGTH;
  conn->negnpn = CURL_HTTP_VERSION_1_1;

  return SSL_TLSEXT_ERR_OK;
}
