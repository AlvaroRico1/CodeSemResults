// Source: curl/lib/url.c
// Lines 1659-1670
static struct connectdata *allocate_conn(struct Curl_easy *data)
{
  struct connectdata *conn = calloc(1, sizeof(struct connectdata));
  if(!conn)
    return NULL;

#ifdef USE_SSL
  /* The SSL backend-specific data (ssl_backend_data) objects are allocated as
     a separate array to ensure suitable alignment.
     Note that these backend pointers can be swapped by vtls (eg ssl backend
     data becomes proxy backend data). */
  {
    size_t sslsize = Curl_ssl->sizeof_ssl_backend_data;
    char *ssl = calloc(4, sslsize);
    if(!ssl) {
      free(conn);
      return NULL;
    }
    conn->ssl_extra = ssl;
    conn->ssl[0].backend = (void *)ssl;
    conn->ssl[1].backend = (void *)(ssl + sslsize);
#ifndef CURL_DISABLE_PROXY
    conn->proxy_ssl[0].backend = (void *)(ssl + 2 * sslsize);
    conn->proxy_ssl[1].backend = (void *)(ssl + 3 * sslsize);
#endif
  }
