CURLcode Curl_hsts_parse(struct hsts *h, const char *hostname,
                         const char *header)
{
  const char *p = header;
  curl_off_t expires = 0;
  bool gotma = FALSE;
  bool gotinc = FALSE;
  bool subdomains = FALSE;
  struct stsentry *sts;
  time_t now = time(NULL);

  if(Curl_host_is_ipnum(hostname))
    /* "explicit IP address identification of all forms is excluded."
       / RFC 6797 */
    return CURLE_OK;

  do {
    while(*p && ISSPACE(*p))
      p++;
    if(Curl_strncasecompare("max-age=", p, 8)) {
      bool quoted = FALSE;
      CURLofft offt;
      char *endp;

      if(gotma)
        return CURLE_BAD_FUNCTION_ARGUMENT;

      p += 8;
      while(*p && ISSPACE(*p))
        p++;
      if(*p == '\"') {
        p++;
        quoted = TRUE;
      }
      offt = curlx_strtoofft(p, &endp, 10, &expires);
      if(offt == CURL_OFFT_FLOW)
        expires = CURL_OFF_T_MAX;
      else if(offt)
        /* invalid max-age */
        return CURLE_BAD_FUNCTION_ARGUMENT;
      p = endp;
      if(quoted) {
        if(*p != '\"')
          return CURLE_BAD_FUNCTION_ARGUMENT;
        p++;
      }
      gotma = TRUE;
    }


// Source: hsts.c
// Lines 130-177
