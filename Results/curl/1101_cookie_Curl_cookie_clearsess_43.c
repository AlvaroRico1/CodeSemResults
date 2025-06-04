void Curl_cookie_clearsess(struct CookieInfo *cookies)
{
  struct Cookie *first, *curr, *next, *prev = NULL;
  unsigned int i;

  if(!cookies)
    return;

  for(i = 0; i < COOKIE_HASH_SIZE; i++) {
    if(!cookies->cookies[i])
      continue;

    first = curr = prev = cookies->cookies[i];

    for(; curr; curr = next) {
      next = curr->next;
      if(!curr->expires) {
        if(first == curr)
          first = next;

        if(prev == curr)
          prev = next;
        else
          prev->next = next;

        freecookie(curr);
        cookies->numcookies--;
      }
      else
        prev = curr;
    }

    cookies->cookies[i] = first;
  }
}


// Source: cookie.c
// Lines 1490-1524
