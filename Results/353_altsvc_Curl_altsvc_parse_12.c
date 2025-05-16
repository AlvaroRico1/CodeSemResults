CURLcode Curl_altsvc_parse(struct Curl_easy *data,
                           struct altsvcinfo *asi, const char *value,
                           enum alpnid srcalpnid, const char *srchost,
                           unsigned short srcport)
{
  const char *p = value;
  size_t len;
  char namebuf[MAX_ALTSVC_HOSTLEN] = "";
  char alpnbuf[MAX_ALTSVC_ALPNLEN] = "";
  struct altsvc *as;
  unsigned short dstport = srcport; /* the same by default */
  CURLcode result = getalnum(&p, alpnbuf, sizeof(alpnbuf));
#ifdef CURL_DISABLE_VERBOSE_STRINGS
  (void)data;
#endif
  if(result) {
    infof(data, "Excessive alt-svc header, ignoring.");
    return CURLE_OK;
  }

  DEBUGASSERT(asi);

  /* Flush all cached alternatives for this source origin, if any */
  altsvc_flush(asi, srcalpnid, srchost, srcport);

  /* "clear" is a magic keyword */
  if(strcasecompare(alpnbuf, "clear")) {
    return CURLE_OK;
  }

  do {
    if(*p == '=') {
      /* [protocol]="[host][:port]" */
      enum alpnid dstalpnid = alpn2alpnid(alpnbuf); /* the same by default */
      p++;
      if(*p == '\"') {
        const char *dsthost = "";
        const char *value_ptr;
        char option[32];
        unsigned long num;
        char *end_ptr;
        bool quoted = FALSE;
        time_t maxage = 24 * 3600; /* default is 24 hours */
        bool persist = FALSE;
        p++;
        if(*p != ':') {
          /* host name starts here */
          const char *hostp = p;
          while(*p && (ISALNUM(*p) || (*p == '.') || (*p == '-')))
            p++;
          len = p - hostp;
          if(!len || (len >= MAX_ALTSVC_HOSTLEN)) {
            infof(data, "Excessive alt-svc host name, ignoring.");
            dstalpnid = ALPN_none;
          }
          else {
            memcpy(namebuf, hostp, len);
            namebuf[len] = 0;
            dsthost = namebuf;
          }
        }


// Source: altsvc.c
// Lines 447-507
