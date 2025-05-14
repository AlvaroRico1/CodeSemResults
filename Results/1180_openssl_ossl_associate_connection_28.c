// Source: curl/lib/vtls/openssl.c
// Lines 4496-4497
static void ossl_associate_connection(struct Curl_easy *data,
                                      struct connectdata *conn,
                                      int sockindex)
{
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  struct ssl_backend_data *backend = connssl->backend;

  /* If we don't have SSL context, do nothing. */
  if(!backend->handle)
    return;

  if(SSL_SET_OPTION(primary.sessionid)) {
    int data_idx = ossl_get_ssl_data_index();
    int connectdata_idx = ossl_get_ssl_conn_index();
    int sockindex_idx = ossl_get_ssl_sockindex_index();
    int proxy_idx = ossl_get_proxy_index();

    if(data_idx >= 0 && connectdata_idx >= 0 && sockindex_idx >= 0 &&
       proxy_idx >= 0) {
      /* Store the data needed for the "new session" callback.
       * The sockindex is stored as a pointer to an array element. */
      SSL_set_ex_data(backend->handle, data_idx, data);
      SSL_set_ex_data(backend->handle, connectdata_idx, conn);
      SSL_set_ex_data(backend->handle, sockindex_idx, conn->sock + sockindex);
#ifndef CURL_DISABLE_PROXY
      SSL_set_ex_data(backend->handle, proxy_idx, SSL_IS_PROXY() ? (void *) 1:
                      NULL);
#else
      SSL_set_ex_data(backend->handle, proxy_idx, NULL);
#endif
    }
  }
}
