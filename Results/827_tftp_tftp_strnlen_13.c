// Source: curl/lib/tftp.c
// Lines 282-284
static size_t tftp_strnlen(const char *string, size_t maxlen)
{
  const char *end = memchr(string, '\0', maxlen);
  return end ? (size_t) (end - string) : maxlen;
}
