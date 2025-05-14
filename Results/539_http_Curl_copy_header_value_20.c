// Source: curl/lib/http.c
// Lines 248-250
char *Curl_copy_header_value(const char *header)
{
  const char *start;
  const char *end;
  char *value;
  size_t len;

  /* Find the end of the header name */
  while(*header && (*header != ':'))
    ++header;

  if(*header)
    /* Skip over colon */
    ++header;

  /* Find the first non-space letter */
  start = header;
  while(*start && ISSPACE(*start))
    start++;

  /* data is in the host encoding so
     use '\r' and '\n' instead of 0x0d and 0x0a */
  end = strchr(start, '\r');
  if(!end)
    end = strchr(start, '\n');
  if(!end)
    end = strchr(start, '\0');
  if(!end)
    return NULL;

  /* skip all trailing space letters */
  while((end > start) && ISSPACE(*end))
    end--;

  /* get length of the type */
  len = end - start + 1;

  value = malloc(len + 1);
  if(!value)
    return NULL;

  memcpy(value, start, len);
  value[len] = 0; /* null-terminate */

  return value;
}
