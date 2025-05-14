// Source: curl/lib/curl_memrchr.c
// Lines 45-48
Curl_memrchr(const void *s, int c, size_t n)
{
  if(n > 0) {
    const unsigned char *p = s;
    const unsigned char *q = s;

    p += n - 1;

    while(p >= q) {
      if(*p == (unsigned char)c)
        return (void *)p;
      p--;
    }
  }
