// Source: curl/lib/curl_addrinfo.c
// Lines 81-84
Curl_freeaddrinfo(struct Curl_addrinfo *cahead)
{
  struct Curl_addrinfo *vqualifier canext;
  struct Curl_addrinfo *ca;

  for(ca = cahead; ca; ca = canext) {
    canext = ca->ai_next;
    free(ca);
  }
}
