// Source: curl/lib/altsvc.c
// Lines 382-383
static CURLcode getalnum(const char **ptr, char *alpnbuf, size_t buflen)
{
  size_t len;
  const char *protop;
  const char *p = *ptr;
  while(*p && ISBLANK(*p))
    p++;
  protop = p;
  while(*p && !ISBLANK(*p) && (*p != ';') && (*p != '='))
    p++;
  len = p - protop;
  *ptr = p;

  if(!len || (len >= buflen))
    return CURLE_BAD_FUNCTION_ARGUMENT;
  memcpy(alpnbuf, protop, len);
  alpnbuf[len] = 0;
  return CURLE_OK;
}
