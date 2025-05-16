CURLcode Curl_doh_is_resolved(struct Curl_easy *data,
                              struct Curl_dns_entry **dnsp)
{
  CURLcode result;
  struct dohdata *dohp = data->req.doh;
  *dnsp = NULL; /* defaults to no response */
  if(!dohp)
    return CURLE_OUT_OF_MEMORY;

  if(!dohp->probe[DOH_PROBE_SLOT_IPADDR_V4].easy &&
     !dohp->probe[DOH_PROBE_SLOT_IPADDR_V6].easy) {
    failf(data, "Could not DoH-resolve: %s", data->state.async.hostname);
    return data->conn->bits.proxy?CURLE_COULDNT_RESOLVE_PROXY:
      CURLE_COULDNT_RESOLVE_HOST;
  }
  else if(!dohp->pending) {
    DOHcode rc[DOH_PROBE_SLOTS] = {
      DOH_OK, DOH_OK
    };
    struct dohentry de;
    int slot;
    /* remove DoH handles from multi handle and close them */
    for(slot = 0; slot < DOH_PROBE_SLOTS; slot++) {
      curl_multi_remove_handle(data->multi, dohp->probe[slot].easy);
      Curl_close(&dohp->probe[slot].easy);
    }
    /* parse the responses, create the struct and return it! */
    de_init(&de);
    for(slot = 0; slot < DOH_PROBE_SLOTS; slot++) {
      struct dnsprobe *p = &dohp->probe[slot];
      if(!p->dnstype)
        continue;
      rc[slot] = doh_decode(Curl_dyn_uptr(&p->serverdoh),
                            Curl_dyn_len(&p->serverdoh),
                            p->dnstype,
                            &de);
      Curl_dyn_free(&p->serverdoh);
      if(rc[slot]) {
        infof(data, "DoH: %s type %s for %s", doh_strerror(rc[slot]),
              type2name(p->dnstype), dohp->host);
      }
    } /* next slot */

    result = CURLE_COULDNT_RESOLVE_HOST; /* until we know better */
    if(!rc[DOH_PROBE_SLOT_IPADDR_V4] || !rc[DOH_PROBE_SLOT_IPADDR_V6]) {
      /* we have an address, of one kind or other */
      struct Curl_dns_entry *dns;
      struct Curl_addrinfo *ai;

      infof(data, "DoH Host name: %s", dohp->host);
      showdoh(data, &de);

      ai = doh2ai(&de, dohp->host, dohp->port);
      if(!ai) {
        de_cleanup(&de);
        return CURLE_OUT_OF_MEMORY;
      }

      if(data->share)
        Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);

      /* we got a response, store it in the cache */
      dns = Curl_cache_addr(data, ai, dohp->host, dohp->port);

      if(data->share)
        Curl_share_unlock(data, CURL_LOCK_DATA_DNS);

      if(!dns) {
        /* returned failure, bail out nicely */
        Curl_freeaddrinfo(ai);
      }
      else {
        data->state.async.dns = dns;
        *dnsp = dns;
        result = CURLE_OK;      /* address resolution OK */
      }
    } /* address processing done */


// Source: doh.c
// Lines 923-999
