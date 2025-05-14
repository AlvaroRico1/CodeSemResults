// Source: curl/lib/vtls/openssl.c
// Lines 4483-4487
static void *ossl_get_internals(struct ssl_connect_data *connssl,
                                CURLINFO info)
{
  /* Legacy: CURLINFO_TLS_SESSION must return an SSL_CTX pointer. */
  struct ssl_backend_data *backend = connssl->backend;
  return info == CURLINFO_TLS_SESSION ?
         (void *)backend->ctx : (void *)backend->handle;
}
