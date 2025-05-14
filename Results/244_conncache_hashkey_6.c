// Source: curl/lib/conncache.c
// Lines 142-144
static void hashkey(struct connectdata *conn, char *buf,
                    size_t len,  /* something like 128 is fine */
                    const char **hostp)
{
  const char *hostname;
  long port = conn->remote_port;

#ifndef CURL_DISABLE_PROXY
  if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
    hostname = conn->http_proxy.host.name;
    port = conn->port;
  }
  else
#endif
    if(conn->bits.conn_to_host)
      hostname = conn->conn_to_host.name;
  else
    hostname = conn->host.name;

  if(hostp)
    /* report back which name we used */
    *hostp = hostname;

  /* put the number first so that the hostname gets cut off if too long */
  msnprintf(buf, len, "%ld%s", port, hostname);
  Curl_strntolower(buf, buf, len);
}
