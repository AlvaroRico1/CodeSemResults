// Source: curl/lib/curl_addrinfo.c
// Lines 383-385
Curl_ip2addr(int af, const void *inaddr, const char *hostname, int port)
{
  struct Curl_addrinfo *ai;

#if defined(__VMS) && \
    defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
#pragma pointer_size save
#pragma pointer_size short
#pragma message disable PTRMISMATCH
#endif

  struct hostent  *h;
  struct namebuff *buf;
  char  *addrentry;
  char  *hoststr;
  size_t addrsize;

  DEBUGASSERT(inaddr && hostname);

  buf = malloc(sizeof(struct namebuff));
  if(!buf)
    return NULL;

  hoststr = strdup(hostname);
  if(!hoststr) {
    free(buf);
    return NULL;
  }

  switch(af) {
  case AF_INET:
    addrsize = sizeof(struct in_addr);
    addrentry = (void *)&buf->addrentry.ina4;
    memcpy(addrentry, inaddr, sizeof(struct in_addr));
    break;
#ifdef ENABLE_IPV6
  case AF_INET6:
    addrsize = sizeof(struct in6_addr);
    addrentry = (void *)&buf->addrentry.ina6;
    memcpy(addrentry, inaddr, sizeof(struct in6_addr));
    break;
#endif
  default:
    free(hoststr);
    free(buf);
    return NULL;
  }

  h = &buf->hostentry;
  h->h_name = hoststr;
  h->h_aliases = NULL;
  h->h_addrtype = (short)af;
  h->h_length = (short)addrsize;
  h->h_addr_list = &buf->h_addr_list[0];
  h->h_addr_list[0] = addrentry;
  h->h_addr_list[1] = NULL; /* terminate list of entries */

#if defined(__VMS) && \
    defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
#pragma pointer_size restore
#pragma message enable PTRMISMATCH
#endif

  ai = Curl_he2ai(h, port);

  free(hoststr);
  free(buf);

  return ai;
}
