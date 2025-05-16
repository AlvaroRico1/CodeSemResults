static char *inet_ntop6 (const unsigned char *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
  char *tp;
  struct {
    long base;
    long len;
  } best, cur;
  unsigned long words[IN6ADDRSZ / INT16SZ];
  int i;

  /* Preprocess:
   *  Copy the input (bytewise) array into a wordwise array.
   *  Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset(words, '\0', sizeof(words));
  for(i = 0; i < IN6ADDRSZ; i++)
    words[i/2] |= (src[i] << ((1 - (i % 2)) << 3));

  best.base = -1;
  cur.base  = -1;
  best.len = 0;
  cur.len = 0;

  for(i = 0; i < (IN6ADDRSZ / INT16SZ); i++) {
    if(words[i] == 0) {
      if(cur.base == -1)
        cur.base = i, cur.len = 1;
      else
        cur.len++;
    }
    else if(cur.base != -1) {
      if(best.base == -1 || cur.len > best.len)
        best = cur;
      cur.base = -1;
    }
  }
  if((cur.base != -1) && (best.base == -1 || cur.len > best.len))
    best = cur;
  if(best.base != -1 && best.len < 2)
    best.base = -1;
  /* Format the result. */
  tp = tmp;
  for(i = 0; i < (IN6ADDRSZ / INT16SZ); i++) {
    /* Are we inside the best run of 0x00's? */
    if(best.base != -1 && i >= best.base && i < (best.base + best.len)) {
      if(i == best.base)
        *tp++ = ':';
      continue;
    }

    /* Are we following an initial run of 0x00s or any real hex?
     */
    if(i)
      *tp++ = ':';

    /* Is this address an encapsulated IPv4?
     */
    if(i == 6 && best.base == 0 &&
        (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
      if(!inet_ntop4(src + 12, tp, sizeof(tmp) - (tp - tmp))) {
        errno = ENOSPC;
        return (NULL);
      }
      tp += strlen(tp);
      break;
    }
    tp += msnprintf(tp, 5, "%lx", words[i]);
  }

  /* Was it a trailing run of 0x00's?
   */
  if(best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
     *tp++ = ':';
  *tp++ = '\0';

  /* Check for overflow, copy, and we're done.
   */
  if((size_t)(tp - tmp) > size) {
    errno = ENOSPC;
    return (NULL);
  }
  strcpy(dst, tmp);
  return dst;
}


// Source: inet_ntop.c
// Lines 77-168
