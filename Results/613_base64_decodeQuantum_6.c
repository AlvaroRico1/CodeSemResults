// Source: curl/lib/base64.c
// Lines 53-56
static size_t decodeQuantum(unsigned char *dest, const char *src)
{
  size_t padding = 0;
  const char *s, *p;
  unsigned long i, x = 0;

  for(i = 0, s = src; i < 4; i++, s++) {
    if(*s == '=') {
      x = (x << 6);
      padding++;
    }
    else {
      unsigned long v = 0;
      p = base64;

      while(*p && (*p != *s)) {
        v++;
        p++;
      }

      if(*p == *s)
        x = (x << 6) + v;
      else
        return 0;
    }
  }

  if(padding < 1)
    dest[2] = curlx_ultouc(x & 0xFFUL);

  x >>= 8;
  if(padding < 2)
    dest[1] = curlx_ultouc(x & 0xFFUL);

  x >>= 8;
  dest[0] = curlx_ultouc(x & 0xFFUL);

  return 3 - padding;
}
