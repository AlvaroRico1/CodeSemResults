static char *auth_digest_string_quoted(const char *source)
{
  char *dest;
  const char *s = source;
  size_t n = 1; /* null terminator */

  /* Calculate size needed */
  while(*s) {
    ++n;
    if(*s == '"' || *s == '\\') {
      ++n;
    }
    ++s;
  }

  dest = malloc(n);
  if(dest) {
    char *d = dest;
    s = source;
    while(*s) {
      if(*s == '"' || *s == '\\') {
        *d++ = '\\';
      }
      *d++ = *s++;
    }
    *d = 0;
  }

  return dest;
}


// Source: digest.c
// Lines 161-190
