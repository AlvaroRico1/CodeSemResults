static void remove_expired(struct CookieInfo *cookies)
{
  struct Cookie *co, *nx;
  curl_off_t now = (curl_off_t)time(NULL);
  unsigned int i;

  /*
   * If the earliest expiration timestamp in the jar is in the future we can
   * skip scanning the whole jar and instead exit early as there won't be any
   * cookies to evict.  If we need to evict however, reset the next_expiration
   * counter in order to track the next one. In case the recorded first
   * expiration is the max offset, then perform the safe fallback of checking
   * all cookies.
   */
  if(now < cookies->next_expiration &&
      cookies->next_expiration != CURL_OFF_T_MAX)
    return;
  else
    cookies->next_expiration = CURL_OFF_T_MAX;

  for(i = 0; i < COOKIE_HASH_SIZE; i++) {
    struct Cookie *pv = NULL;
    co = cookies->cookies[i];
    while(co) {
      nx = co->next;
      if(co->expires && co->expires < now) {
        if(!pv) {
          cookies->cookies[i] = co->next;
        }
        else {
          pv->next = co->next;
        }
        cookies->numcookies--;
        freecookie(co);
      }
      else {
        /*
         * If this cookie has an expiration timestamp earlier than what we've
         * seen so far then record it for the next round of expirations.
         */
        if(co->expires && co->expires < cookies->next_expiration)
          cookies->next_expiration = co->expires;
        pv = co;
      }
      co = nx;
    }
  }
}


// Source: cookie.c
// Lines 378-425
