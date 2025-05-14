// Source: curl/lib/hostip.c
// Lines 605-607
enum resolve_t Curl_resolv(struct Curl_easy *data,
                           const char *hostname,
                           int port,
                           bool allowDOH,
                           struct Curl_dns_entry **entry)
{
  struct Curl_dns_entry *dns = NULL;
  CURLcode result;
  enum resolve_t rc = CURLRESOLV_ERROR; /* default to failure */
  struct connectdata *conn = data->conn;
  *entry = NULL;
  conn->bits.doh = FALSE; /* default is not */

  if(data->share)
    Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);

  dns = fetch_addr(data, hostname, port);

  if(dns) {
    infof(data, "Hostname %s was found in DNS cache", hostname);
    dns->inuse++; /* we use it! */
    rc = CURLRESOLV_RESOLVED;
  }

  if(data->share)
    Curl_share_unlock(data, CURL_LOCK_DATA_DNS);

  if(!dns) {
    /* The entry was not in the cache. Resolve it to IP address */

    struct Curl_addrinfo *addr = NULL;
    int respwait = 0;
    struct in_addr in;
#ifndef USE_RESOLVE_ON_IPS
    const
#endif
      bool ipnum = FALSE;

    /* notify the resolver start callback */
    if(data->set.resolver_start) {
      int st;
      Curl_set_in_callback(data, true);
      st = data->set.resolver_start(
#ifdef USE_CURL_ASYNC
        data->state.async.resolver,
#else
        NULL,
#endif
        NULL,
        data->set.resolver_start_client);
      Curl_set_in_callback(data, false);
      if(st)
        return CURLRESOLV_ERROR;
    }

#if defined(ENABLE_IPV6) && defined(CURL_OSX_CALL_COPYPROXIES)
    {
      /*
       * The automagic conversion from IPv4 literals to IPv6 literals only
       * works if the SCDynamicStoreCopyProxies system function gets called
       * first. As Curl currently doesn't support system-wide HTTP proxies, we
       * therefore don't use any value this function might return.
       *
       * This function is only available on a macOS and is not needed for
       * IPv4-only builds, hence the conditions above.
       */
      CFDictionaryRef dict = SCDynamicStoreCopyProxies(NULL);
      if(dict)
        CFRelease(dict);
    }
#endif

#ifndef USE_RESOLVE_ON_IPS
    /* First check if this is an IPv4 address string */
    if(Curl_inet_pton(AF_INET, hostname, &in) > 0)
      /* This is a dotted IP address 123.123.123.123-style */
      addr = Curl_ip2addr(AF_INET, &in, hostname, port);
#ifdef ENABLE_IPV6
    if(!addr) {
      struct in6_addr in6;
      /* check if this is an IPv6 address string */
      if(Curl_inet_pton(AF_INET6, hostname, &in6) > 0)
        /* This is an IPv6 address literal */
        addr = Curl_ip2addr(AF_INET6, &in6, hostname, port);
    }
#endif /* ENABLE_IPV6 */

#else /* if USE_RESOLVE_ON_IPS */
    /* First check if this is an IPv4 address string */
    if(Curl_inet_pton(AF_INET, hostname, &in) > 0)
      /* This is a dotted IP address 123.123.123.123-style */
      ipnum = TRUE;
#ifdef ENABLE_IPV6
    else {
      struct in6_addr in6;
      /* check if this is an IPv6 address string */
      if(Curl_inet_pton(AF_INET6, hostname, &in6) > 0)
        /* This is an IPv6 address literal */
        ipnum = TRUE;
    }
#endif /* ENABLE_IPV6 */

#endif /* !USE_RESOLVE_ON_IPS */

    if(!addr) {
      if(conn->ip_version == CURL_IPRESOLVE_V6 && !Curl_ipv6works(data))
        return CURLRESOLV_ERROR;

      if(strcasecompare(hostname, "localhost"))
        addr = get_localhost(port);
      else if(allowDOH && data->set.doh && !ipnum)
        addr = Curl_doh(data, hostname, port, &respwait);
      else {
        /* Check what IP specifics the app has requested and if we can provide
         * it. If not, bail out. */
        if(!Curl_ipvalid(data, conn))
          return CURLRESOLV_ERROR;
        /* If Curl_getaddrinfo() returns NULL, 'respwait' might be set to a
           non-zero value indicating that we need to wait for the response to
           the resolve call */
        addr = Curl_getaddrinfo(data, hostname, port, &respwait);
      }
    }
    if(!addr) {
      if(respwait) {
        /* the response to our resolve call will come asynchronously at
           a later time, good or bad */
        /* First, check that we haven't received the info by now */
        result = Curl_resolv_check(data, &dns);
        if(result) /* error detected */
          return CURLRESOLV_ERROR;
        if(dns)
          rc = CURLRESOLV_RESOLVED; /* pointer provided */
        else
          rc = CURLRESOLV_PENDING; /* no info yet */
      }
    }
    else {
      if(data->share)
        Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);

      /* we got a response, store it in the cache */
      dns = Curl_cache_addr(data, addr, hostname, port);

      if(data->share)
        Curl_share_unlock(data, CURL_LOCK_DATA_DNS);

      if(!dns)
        /* returned failure, bail out nicely */
        Curl_freeaddrinfo(addr);
      else
        rc = CURLRESOLV_RESOLVED;
    }
  }

  *entry = dns;

  return rc;
}
