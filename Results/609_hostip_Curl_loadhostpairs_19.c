CURLcode Curl_loadhostpairs(struct Curl_easy *data)
{
  struct curl_slist *hostp;
  char hostname[256];
  int port = 0;

  /* Default is no wildcard found */
  data->state.wildcard_resolve = false;

  for(hostp = data->state.resolve; hostp; hostp = hostp->next) {
    char entry_id[MAX_HOSTCACHE_LEN];
    if(!hostp->data)
      continue;
    if(hostp->data[0] == '-') {
      size_t entry_len;

      if(2 != sscanf(hostp->data + 1, "%255[^:]:%d", hostname, &port)) {
        infof(data, "Couldn't parse CURLOPT_RESOLVE removal entry '%s'",
              hostp->data);
        continue;
      }

      /* Create an entry id, based upon the hostname and port */
      create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
      entry_len = strlen(entry_id);

      if(data->share)
        Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);

      /* delete entry, ignore if it didn't exist */
      Curl_hash_delete(data->dns.hostcache, entry_id, entry_len + 1);

      if(data->share)
        Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
    }
    else {
      struct Curl_dns_entry *dns;
      struct Curl_addrinfo *head = NULL, *tail = NULL;
      size_t entry_len;
      char address[64];
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
      char *addresses = NULL;
#endif
      char *addr_begin;
      char *addr_end;
      char *port_ptr;
      char *end_ptr;
      bool permanent = TRUE;
      char *host_begin;
      char *host_end;
      unsigned long tmp_port;
      bool error = true;

      host_begin = hostp->data;
      if(host_begin[0] == '+') {
        host_begin++;
        permanent = FALSE;
      }
      host_end = strchr(host_begin, ':');
      if(!host_end ||
         ((host_end - host_begin) >= (ptrdiff_t)sizeof(hostname)))
        goto err;

      memcpy(hostname, host_begin, host_end - host_begin);
      hostname[host_end - host_begin] = '\0';

      port_ptr = host_end + 1;
      tmp_port = strtoul(port_ptr, &end_ptr, 10);
      if(tmp_port > USHRT_MAX || end_ptr == port_ptr || *end_ptr != ':')
        goto err;

      port = (int)tmp_port;
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
      addresses = end_ptr + 1;
#endif

      while(*end_ptr) {
        size_t alen;
        struct Curl_addrinfo *ai;

        addr_begin = end_ptr + 1;
        addr_end = strchr(addr_begin, ',');
        if(!addr_end)
          addr_end = addr_begin + strlen(addr_begin);
        end_ptr = addr_end;

        /* allow IP(v6) address within [brackets] */
        if(*addr_begin == '[') {
          if(addr_end == addr_begin || *(addr_end - 1) != ']')
            goto err;
          ++addr_begin;
          --addr_end;
        }

        alen = addr_end - addr_begin;
        if(!alen)
          continue;

        if(alen >= sizeof(address))
          goto err;

        memcpy(address, addr_begin, alen);
        address[alen] = '\0';

#ifndef ENABLE_IPV6
        if(strchr(address, ':')) {
          infof(data, "Ignoring resolve address '%s', missing IPv6 support.",
                address);
          continue;
        }
#endif

        ai = Curl_str2addr(address, port);
        if(!ai) {
          infof(data, "Resolve address '%s' found illegal!", address);
          goto err;
        }

        if(tail) {
          tail->ai_next = ai;
          tail = tail->ai_next;
        }
        else {
          head = tail = ai;
        }
      }

      if(!head)
        goto err;

      error = false;
   err:
      if(error) {
        failf(data, "Couldn't parse CURLOPT_RESOLVE entry '%s'!",
              hostp->data);
        Curl_freeaddrinfo(head);
        return CURLE_SETOPT_OPTION_SYNTAX;
      }

      /* Create an entry id, based upon the hostname and port */
      create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
      entry_len = strlen(entry_id);

      if(data->share)
        Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);

      /* See if it's already in our dns cache */
      dns = Curl_hash_pick(data->dns.hostcache, entry_id, entry_len + 1);

      if(dns) {
        infof(data, "RESOLVE %s:%d is - old addresses discarded!",
                hostname, port);
        /* delete old entry, there are two reasons for this
         1. old entry may have different addresses.
         2. even if entry with correct addresses is already in the cache,
            but if it is close to expire, then by the time next http
            request is made, it can get expired and pruned because old
            entry is not necessarily marked as permanent.
         3. when adding a non-permanent entry, we want it to remove and
            replace an existing permanent entry.
         4. when adding a non-permanent entry, we want it to get a "fresh"
            timeout that starts _now_. */

        Curl_hash_delete(data->dns.hostcache, entry_id, entry_len + 1);
      }

      /* put this new host in the cache */
      dns = Curl_cache_addr(data, head, hostname, port);
      if(dns) {
        if(permanent)
          dns->timestamp = 0; /* mark as permanent */
        /* release the returned reference; the cache itself will keep the
         * entry alive: */
        dns->inuse--;
      }

      if(data->share)
        Curl_share_unlock(data, CURL_LOCK_DATA_DNS);

      if(!dns) {
        Curl_freeaddrinfo(head);
        return CURLE_OUT_OF_MEMORY;
      }
      infof(data, "Added %s:%d:%s to DNS cache%s",
            hostname, port, addresses, permanent ? "" : " (non-permanent)");

      /* Wildcard hostname */
      if(hostname[0] == '*' && hostname[1] == '\0') {
        infof(data, "RESOLVE %s:%d is wildcard, enabling wildcard checks",
              hostname, port);
        data->state.wildcard_resolve = true;
      }
    }
  }
  data->state.resolve = NULL; /* dealt with now */

  return CURLE_OK;
}


// Source: hostip.c
// Lines 1008-1205
