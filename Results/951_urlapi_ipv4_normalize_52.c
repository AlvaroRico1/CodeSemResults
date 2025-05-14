// Source: curl/lib/urlapi.c
// Lines 685-689
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
