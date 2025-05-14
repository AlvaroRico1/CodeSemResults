// Source: curl/lib/doh.c
// Lines 394-400
struct Curl_addrinfo *Curl_doh(struct Curl_easy *data,
                               const char *hostname,
                               int port,
                               int *waitp)
{
  CURLcode result = CURLE_OK;
  int slot;
  struct dohdata *dohp;
  struct connectdata *conn = data->conn;
  *waitp = TRUE; /* this never returns synchronously */
  (void)hostname;
  (void)port;

  DEBUGASSERT(!data->req.doh);
  DEBUGASSERT(conn);

  /* start clean, consider allocating this struct on demand */
  dohp = data->req.doh = calloc(sizeof(struct dohdata), 1);
  if(!dohp)
    return NULL;

  conn->bits.doh = TRUE;
  dohp->host = hostname;
  dohp->port = port;
  dohp->headers =
    curl_slist_append(NULL,
                      "Content-Type: application/dns-message");
  if(!dohp->headers)
    goto error;

  /* create IPv4 DoH request */
  result = dohprobe(data, &dohp->probe[DOH_PROBE_SLOT_IPADDR_V4],
                    DNS_TYPE_A, hostname, data->set.str[STRING_DOH],
                    data->multi, dohp->headers);
  if(result)
    goto error;
  dohp->pending++;

  if(Curl_ipv6works(data)) {
    /* create IPv6 DoH request */
    result = dohprobe(data, &dohp->probe[DOH_PROBE_SLOT_IPADDR_V6],
                      DNS_TYPE_AAAA, hostname, data->set.str[STRING_DOH],
                      data->multi, dohp->headers);
    if(result)
      goto error;
    dohp->pending++;
  }
  return NULL;

  error:
  curl_slist_free_all(dohp->headers);
  data->req.doh->headers = NULL;
  for(slot = 0; slot < DOH_PROBE_SLOTS; slot++) {
    Curl_close(&dohp->probe[slot].easy);
  }
  Curl_safefree(data->req.doh);
  return NULL;
}
