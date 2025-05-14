// Source: curl/lib/http_proxy.c
// Lines 221-224
static CURLcode CONNECT_host(struct Curl_easy *data,
                             struct connectdata *conn,
                             const char *hostname,
                             int remote_port,
                             char **connecthostp,
                             char **hostp)
{
  char *hostheader; /* for CONNECT */
  char *host = NULL; /* Host: */
  bool ipv6_ip = conn->bits.ipv6_ip;

  /* the hostname may be different */
  if(hostname != conn->host.name)
    ipv6_ip = (strchr(hostname, ':') != NULL);
  hostheader = /* host:port with IPv6 support */
    aprintf("%s%s%s:%d", ipv6_ip?"[":"", hostname, ipv6_ip?"]":"",
            remote_port);
  if(!hostheader)
    return CURLE_OUT_OF_MEMORY;

  if(!Curl_checkProxyheaders(data, conn, "Host")) {
    host = aprintf("Host: %s\r\n", hostheader);
    if(!host) {
      free(hostheader);
      return CURLE_OUT_OF_MEMORY;
    }
  }
  *connecthostp = hostheader;
  *hostp = host;
  return CURLE_OK;
}
