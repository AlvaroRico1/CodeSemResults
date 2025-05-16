Curl_cache_addr(struct Curl_easy *data,
                struct Curl_addrinfo *addr,
                const char *hostname,
                int port)
{
  char entry_id[MAX_HOSTCACHE_LEN];
  size_t entry_len;
  struct Curl_dns_entry *dns;
  struct Curl_dns_entry *dns2;

#ifndef CURL_DISABLE_SHUFFLE_DNS
  /* shuffle addresses if requested */
  if(data->set.dns_shuffle_addresses) {
    CURLcode result = Curl_shuffle_addr(data, &addr);
    if(result)
      return NULL;
  }
#endif

  /* Create a new cache entry */
  dns = calloc(1, sizeof(struct Curl_dns_entry));
  if(!dns) {
    return NULL;
  }

  /* Create an entry id, based upon the hostname and port */
  create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
  entry_len = strlen(entry_id);

  dns->inuse = 1;   /* the cache has the first reference */
  dns->addr = addr; /* this is the address(es) */
  time(&dns->timestamp);
  if(dns->timestamp == 0)
    dns->timestamp = 1;   /* zero indicates permanent CURLOPT_RESOLVE entry */

  /* Store the resolved data in our DNS cache. */
  dns2 = Curl_hash_add(data->dns.hostcache, entry_id, entry_len + 1,
                       (void *)dns);
  if(!dns2) {
    free(dns);
    return NULL;
  }

  dns = dns2;
  dns->inuse++;         /* mark entry as in-use */
  return dns;
}


// Source: hostip.c
// Lines 415-461
