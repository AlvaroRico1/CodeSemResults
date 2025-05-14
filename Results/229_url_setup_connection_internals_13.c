// Source: curl/lib/url.c
// Lines 2156-2158
static CURLcode setup_connection_internals(struct Curl_easy *data,
                                           struct connectdata *conn)
{
  const struct Curl_handler *p;
  CURLcode result;

  /* Perform setup complement if some. */
  p = conn->handler;

  if(p->setup_connection) {
    result = (*p->setup_connection)(data, conn);

    if(result)
      return result;

    p = conn->handler;              /* May have changed. */
  }

  if(conn->port < 0)
    /* we check for -1 here since if proxy was detected already, this
       was very likely already set to the proxy port */
    conn->port = p->defport;

  return CURLE_OK;
}
