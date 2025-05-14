// Source: curl/lib/hostip.c
// Lines 144-152
void Curl_printable_address(const struct Curl_addrinfo *ai, char *buf,
                            size_t bufsize)
{
  DEBUGASSERT(bufsize);
  buf[0] = 0;

  switch(ai->ai_family) {
  case AF_INET: {
    const struct sockaddr_in *sa4 = (const void *)ai->ai_addr;
    const struct in_addr *ipaddr4 = &sa4->sin_addr;
    (void)Curl_inet_ntop(ai->ai_family, (const void *)ipaddr4, buf, bufsize);
    break;
  }
