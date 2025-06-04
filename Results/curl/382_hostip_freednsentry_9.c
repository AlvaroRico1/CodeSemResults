static void freednsentry(void *freethis)
{
  struct Curl_dns_entry *dns = (struct Curl_dns_entry *) freethis;
  DEBUGASSERT(dns && (dns->inuse>0));

  dns->inuse--;
  if(dns->inuse == 0) {
    Curl_freeaddrinfo(dns->addr);
    free(dns);
  }
}


// Source: hostip.c
// Lines 967-977
