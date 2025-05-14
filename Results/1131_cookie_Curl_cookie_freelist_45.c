// Source: curl/lib/cookie.c
// Lines 1475-1477
void Curl_cookie_freelist(struct Cookie *co)
{
  struct Cookie *next;
  while(co) {
    next = co->next;
    freecookie(co);
    co = next;
  }
}
