static void ossl_disassociate_connection(struct Curl_easy *data,
                                         int sockindex)
{
  struct connectdata *conn = data->conn;
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
      /* Disable references to data in "new session" callback to avoid
       * accessing a stale pointer. */
      SSL_set_ex_data(backend->handle, data_idx, NULL);
      SSL_set_ex_data(backend->handle, connectdata_idx, NULL);
      SSL_set_ex_data(backend->handle, sockindex_idx, NULL);
      SSL_set_ex_data(backend->handle, proxy_idx, NULL);
    }
  }
}


// Source: openssl.c
// Lines 4534-4561
