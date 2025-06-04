static size_t tftp_strnlen(const char *string, size_t maxlen)
{
  const char *end = memchr(string, '\0', maxlen);
  return end ? (size_t) (end - string) : maxlen;
}


// Source: tftp.c
// Lines 282-286
