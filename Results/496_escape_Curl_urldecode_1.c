CURLcode Curl_urldecode(struct Curl_easy *data,
                        const char *string, size_t length,
                        char **ostring, size_t *olen,
                        enum urlreject ctrl)
{
  size_t alloc;
  char *ns;
  size_t strindex = 0;
  unsigned long hex;
  CURLcode result = CURLE_OK;

  DEBUGASSERT(string);
  DEBUGASSERT(ctrl >= REJECT_NADA); /* crash on TRUE/FALSE */

  alloc = (length?length:strlen(string)) + 1;
  ns = malloc(alloc);

  if(!ns)
    return CURLE_OUT_OF_MEMORY;

  while(--alloc > 0) {
    unsigned char in = *string;
    if(('%' == in) && (alloc > 2) &&
       ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
      /* this is two hexadecimal digits following a '%' */
      char hexstr[3];
      char *ptr;
      hexstr[0] = string[1];
      hexstr[1] = string[2];
      hexstr[2] = 0;

      hex = strtoul(hexstr, &ptr, 16);

      in = curlx_ultouc(hex); /* this long is never bigger than 255 anyway */

      if(data) {
        result = Curl_convert_from_network(data, (char *)&in, 1);
        if(result) {
          /* Curl_convert_from_network calls failf if unsuccessful */
          free(ns);
          return result;
        }
      }

      string += 2;
      alloc -= 2;
    }

    if(((ctrl == REJECT_CTRL) && (in < 0x20)) ||
       ((ctrl == REJECT_ZERO) && (in == 0))) {
      free(ns);
      return CURLE_URL_MALFORMAT;
    }

    ns[strindex++] = in;
    string++;
  }
  ns[strindex] = 0; /* terminate it */

  if(olen)
    /* store output size */
    *olen = strindex;

  /* store output string */
  *ostring = ns;

  return CURLE_OK;
}


// Source: escape.c
// Lines 142-209
