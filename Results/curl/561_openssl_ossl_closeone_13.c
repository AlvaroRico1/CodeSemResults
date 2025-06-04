static void ossl_closeone(struct Curl_easy *data,
                          struct connectdata *conn,
                          struct ssl_connect_data *connssl)
{
  struct ssl_backend_data *backend = connssl->backend;
  if(backend->handle) {
    char buf[32];
    set_logger(conn, data);

    /* Maybe the server has already sent a close notify alert.
       Read it to avoid an RST on the TCP connection. */
    (void)SSL_read(backend->handle, buf, (int)sizeof(buf));

    (void)SSL_shutdown(backend->handle);
    SSL_set_connect_state(backend->handle);

    SSL_free(backend->handle);
    backend->handle = NULL;
  }
  if(backend->ctx) {
    SSL_CTX_free(backend->ctx);
    backend->ctx = NULL;
  }
}


// Source: openssl.c
// Lines 1396-1419
