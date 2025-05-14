// Source: curl/lib/altsvc.c
// Lines 449-483
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
        else {
          /* no destination name, use source host */
          dsthost = srchost;
        }
        if(*p == ':') {
          /* a port number */
          unsigned long port = strtoul(++p, &end_ptr, 10);
          if(port > USHRT_MAX || end_ptr == p || *end_ptr != '\"') {
            infof(data, "Unknown alt-svc port number, ignoring.");
            dstalpnid = ALPN_none;
          }
          p = end_ptr;
          dstport = curlx_ultous(port);
        }
        if(*p++ != '\"')
          break;
        /* Handle the optional 'ma' and 'persist' flags. Unknown flags
           are skipped. */
        for(;;) {
          while(ISBLANK(*p))
            p++;
          if(*p != ';')
            break;
          p++; /* pass the semicolon */
          if(!*p || ISNEWLINE(*p))
            break;
          result = getalnum(&p, option, sizeof(option));
          if(result) {
            /* skip option if name is too long */
            option[0] = '\0';
          }
          while(*p && ISBLANK(*p))
            p++;
          if(*p != '=')
            return CURLE_OK;
          p++;
          while(*p && ISBLANK(*p))
            p++;
          if(!*p)
            return CURLE_OK;
          if(*p == '\"') {
            /* quoted value */
            p++;
            quoted = TRUE;
          }
          value_ptr = p;
          if(quoted) {
            while(*p && *p != '\"')
              p++;
            if(!*p++)
              return CURLE_OK;
          }
          else {
            while(*p && !ISBLANK(*p) && *p!= ';' && *p != ',')
              p++;
          }
          num = strtoul(value_ptr, &end_ptr, 10);
          if((end_ptr != value_ptr) && (num < ULONG_MAX)) {
            if(strcasecompare("ma", option))
              maxage = num;
            else if(strcasecompare("persist", option) && (num == 1))
              persist = TRUE;
          }
        }
        if(dstalpnid) {
          as = altsvc_createid(srchost, dsthost,
                               srcalpnid, dstalpnid,
                               srcport, dstport);
          if(as) {
            /* The expires time also needs to take the Age: value (if any) into
               account. [See RFC 7838 section 3.1] */
            as->expires = maxage + time(NULL);
            as->persist = persist;
            Curl_llist_insert_next(&asi->list, asi->list.tail, as, &as->node);
            infof(data, "Added alt-svc: %s:%d over %s", dsthost, dstport,
                  Curl_alpnid2str(dstalpnid));
          }
        }
        else {
          infof(data, "Unknown alt-svc protocol \"%s\", skipping.",
                alpnbuf);
        }
      }
