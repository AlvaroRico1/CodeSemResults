// Source: curl/lib/cookie.c
// Lines 480-508
Curl_cookie_add(struct Curl_easy *data,
                /*
                 * The 'data' pointer here may be NULL at times, and thus
                 * must only be used very carefully for things that can deal
                 * with data being NULL. Such as infof() and similar
                 */
                struct CookieInfo *c,
                bool httpheader, /* TRUE if HTTP header-style line */
                bool noexpire, /* if TRUE, skip remove_expired() */
                char *lineptr,   /* first character of the line */
                const char *domain, /* default domain */
                const char *path,   /* full path used when this cookie is set,
                                       used to get default path for the cookie
                                       unless set */
                bool secure)  /* TRUE if connection is over secure origin */
{
  struct Cookie *clist;
  struct Cookie *co;
  struct Cookie *lastc = NULL;
  time_t now = time(NULL);
  bool replace_old = FALSE;
  bool badcookie = FALSE; /* cookies are good by default. mmmmm yummy */
  size_t myhash;

#ifdef CURL_DISABLE_VERBOSE_STRINGS
  (void)data;
#endif

  /* First, alloc and init a new struct for it */
  co = calloc(1, sizeof(struct Cookie));
  if(!co)
    return NULL; /* bail out if we're this low on memory */

  if(httpheader) {
    /* This line was read off a HTTP-header */
    char name[MAX_NAME];
    char what[MAX_NAME];
    const char *ptr;
    const char *semiptr;

    size_t linelength = strlen(lineptr);
    if(linelength > MAX_COOKIE_LINE) {
      /* discard overly long lines at once */
      free(co);
      return NULL;
    }

    semiptr = strchr(lineptr, ';'); /* first, find a semicolon */

    while(*lineptr && ISBLANK(*lineptr))
      lineptr++;

    ptr = lineptr;
    do {
      /* we have a <what>=<this> pair or a stand-alone word here */
      name[0] = what[0] = 0; /* init the buffers */
      if(1 <= sscanf(ptr, "%" MAX_NAME_TXT "[^;\r\n=] =%"
                     MAX_NAME_TXT "[^;\r\n]",
                     name, what)) {
        /*
         * Use strstore() below to properly deal with received cookie
         * headers that have the same string property set more than once,
         * and then we use the last one.
         */
        const char *whatptr;
        bool done = FALSE;
        bool sep;
        size_t len = strlen(what);
        size_t nlen = strlen(name);
        const char *endofn = &ptr[ nlen ];

        /*
         * Check for too long individual name or contents, or too long
         * combination of name + contents. Chrome and Firefox support 4095 or
         * 4096 bytes combo
         */
        if(nlen >= (MAX_NAME-1) || len >= (MAX_NAME-1) ||
           ((nlen + len) > MAX_NAME)) {
          freecookie(co);
          infof(data, "oversized cookie dropped, name/val %zu + %zu bytes",
                nlen, len);
          return NULL;
        }

        /* name ends with a '=' ? */
        sep = (*endofn == '=')?TRUE:FALSE;

        if(nlen) {
          endofn--; /* move to the last character */
          if(ISBLANK(*endofn)) {
            /* skip trailing spaces in name */
            while(*endofn && ISBLANK(*endofn) && nlen) {
              endofn--;
              nlen--;
            }
            name[nlen] = 0; /* new end of name */
          }
        }

        /* Strip off trailing whitespace from the 'what' */
        while(len && ISBLANK(what[len-1])) {
          what[len-1] = 0;
          len--;
        }

        /* Skip leading whitespace from the 'what' */
        whatptr = what;
        while(*whatptr && ISBLANK(*whatptr))
          whatptr++;

        /*
         * Check if we have a reserved prefix set before anything else, as we
         * otherwise have to test for the prefix in both the cookie name and
         * "the rest". Prefixes must start with '__' and end with a '-', so
         * only test for names where that can possibly be true.
         */
        if(nlen > 3 && name[0] == '_' && name[1] == '_') {
          if(!strncmp("__Secure-", name, 9))
            co->prefix |= COOKIE_PREFIX__SECURE;
          else if(!strncmp("__Host-", name, 7))
            co->prefix |= COOKIE_PREFIX__HOST;
        }

        if(!co->name) {
          /* The very first name/value pair is the actual cookie name */
          if(!sep) {
            /* Bad name/value pair. */
            badcookie = TRUE;
            break;
          }
          co->name = strdup(name);
          co->value = strdup(whatptr);
          done = TRUE;
          if(!co->name || !co->value) {
            badcookie = TRUE;
            break;
          }
        }
        else if(!len) {
          /*
           * this was a "<name>=" with no content, and we must allow
           * 'secure' and 'httponly' specified this weirdly
           */
          done = TRUE;
          /*
           * secure cookies are only allowed to be set when the connection is
           * using a secure protocol, or when the cookie is being set by
           * reading from file
           */
          if(strcasecompare("secure", name)) {
            if(secure || !c->running) {
              co->secure = TRUE;
            }
            else {
              badcookie = TRUE;
              break;
            }
          }
          else if(strcasecompare("httponly", name))
            co->httponly = TRUE;
          else if(sep)
            /* there was a '=' so we're not done parsing this field */
            done = FALSE;
        }
        if(done)
          ;
        else if(strcasecompare("path", name)) {
          strstore(&co->path, whatptr);
          if(!co->path) {
            badcookie = TRUE; /* out of memory bad */
            break;
          }
          free(co->spath); /* if this is set again */
          co->spath = sanitize_cookie_path(co->path);
          if(!co->spath) {
            badcookie = TRUE; /* out of memory bad */
            break;
          }
        }
        else if(strcasecompare("domain", name)) {
          bool is_ip;

          /*
           * Now, we make sure that our host is within the given domain, or
           * the given domain is not valid and thus cannot be set.
           */

          if('.' == whatptr[0])
            whatptr++; /* ignore preceding dot */

#ifndef USE_LIBPSL
          /*
           * Without PSL we don't know when the incoming cookie is set on a
           * TLD or otherwise "protected" suffix. To reduce risk, we require a
           * dot OR the exact host name being "localhost".
           */
          if(bad_domain(whatptr))
            domain = ":";
#endif

          is_ip = Curl_host_is_ipnum(domain ? domain : whatptr);

          if(!domain
             || (is_ip && !strcmp(whatptr, domain))
             || (!is_ip && tailmatch(whatptr, domain))) {
            strstore(&co->domain, whatptr);
            if(!co->domain) {
              badcookie = TRUE;
              break;
            }
            if(!is_ip)
              co->tailmatch = TRUE; /* we always do that if the domain name was
                                       given */
          }
          else {
            /*
             * We did not get a tailmatch and then the attempted set domain is
             * not a domain to which the current host belongs. Mark as bad.
             */
            badcookie = TRUE;
            infof(data, "skipped cookie with bad tailmatch domain: %s",
                  whatptr);
          }
        }
        else if(strcasecompare("version", name)) {
          strstore(&co->version, whatptr);
          if(!co->version) {
            badcookie = TRUE;
            break;
          }
        }
        else if(strcasecompare("max-age", name)) {
          /*
           * Defined in RFC2109:
           *
           * Optional.  The Max-Age attribute defines the lifetime of the
           * cookie, in seconds.  The delta-seconds value is a decimal non-
           * negative integer.  After delta-seconds seconds elapse, the
           * client should discard the cookie.  A value of zero means the
           * cookie should be discarded immediately.
           */
          strstore(&co->maxage, whatptr);
          if(!co->maxage) {
            badcookie = TRUE;
            break;
          }
        }
        else if(strcasecompare("expires", name)) {
          strstore(&co->expirestr, whatptr);
          if(!co->expirestr) {
            badcookie = TRUE;
            break;
          }
        }

        /*
         * Else, this is the second (or more) name we don't know about!
         */
      }
