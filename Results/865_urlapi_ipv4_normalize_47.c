// Source: curl/lib/urlapi.c
// Lines 681-685
static bool ipv4_normalize(const char *hostname, char *outp, size_t olen)
{
  bool done = FALSE;
  int n = 0;
  const char *c = hostname;
  unsigned long parts[4] = {0, 0, 0, 0};

  while(!done) {
    char *endp;
    unsigned long l;
    if((*c < '0') || (*c > '9'))
      /* most importantly this doesn't allow a leading plus or minus */
      return FALSE;
    l = strtoul(c, &endp, 0);

    /* overflow or nothing parsed at all */
    if(((l == ULONG_MAX) && (errno == ERANGE)) ||  (endp == c))
      return FALSE;

#if SIZEOF_LONG > 4
    /* a value larger than 32 bits */
    if(l > UINT_MAX)
      return FALSE;
#endif

    parts[n] = l;
    c = endp;

    switch (*c) {
    case '.' :
      if(n == 3)
        return FALSE;
      n++;
      c++;
      break;

    case '\0':
      done = TRUE;
      break;

    default:
      return FALSE;
    }
  }

  /* this is deemed a valid IPv4 numerical address */

  switch(n) {
  case 0: /* a -- 32 bits */
    msnprintf(outp, olen, "%u.%u.%u.%u",
              parts[0] >> 24, (parts[0] >> 16) & 0xff,
              (parts[0] >> 8) & 0xff, parts[0] & 0xff);
    break;
  case 1: /* a.b -- 8.24 bits */
    if((parts[0] > 0xff) || (parts[1] > 0xffffff))
      return FALSE;
    msnprintf(outp, olen, "%u.%u.%u.%u",
              parts[0], (parts[1] >> 16) & 0xff,
              (parts[1] >> 8) & 0xff, parts[1] & 0xff);
    break;
  case 2: /* a.b.c -- 8.8.16 bits */
    if((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xffff))
      return FALSE;
    msnprintf(outp, olen, "%u.%u.%u.%u",
              parts[0], parts[1], (parts[2] >> 8) & 0xff,
              parts[2] & 0xff);
    break;
  case 3: /* a.b.c.d -- 8.8.8.8 bits */
    if((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff) ||
       (parts[3] > 0xff))
      return FALSE;
    msnprintf(outp, olen, "%u.%u.%u.%u",
              parts[0], parts[1], parts[2], parts[3]);
    break;
  }
  return TRUE;
}
