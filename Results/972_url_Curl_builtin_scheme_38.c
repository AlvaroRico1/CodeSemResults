const struct Curl_handler *Curl_builtin_scheme(const char *scheme)
{
  const struct Curl_handler * const *pp;
  const struct Curl_handler *p;
  /* Scan protocol handler table and match against 'scheme'. The handler may
     be changed later when the protocol specific setup function is called. */
  for(pp = protocols; (p = *pp) != NULL; pp++)
    if(strcasecompare(p->scheme, scheme))
      /* Protocol found in table. Check if allowed */
      return p;
  return NULL; /* not found */
}


// Source: url.c
// Lines 1799-1810
