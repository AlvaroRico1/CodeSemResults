CURLUcode curl_url_set(CURLU *u, CURLUPart what,
                       const char *part, unsigned int flags)
{
  char **storep = NULL;
  long port = 0;
  bool urlencode = (flags & CURLU_URLENCODE)? 1 : 0;
  bool plusencode = FALSE;
  bool urlskipslash = FALSE;
  bool appendquery = FALSE;
  bool equalsencode = FALSE;

  if(!u)
    return CURLUE_BAD_HANDLE;
  if(!part) {
    /* setting a part to NULL clears it */
    switch(what) {
    case CURLUPART_URL:
      break;
    case CURLUPART_SCHEME:
      storep = &u->scheme;
      break;
    case CURLUPART_USER:
      storep = &u->user;
      break;
    case CURLUPART_PASSWORD:
      storep = &u->password;
      break;
    case CURLUPART_OPTIONS:
      storep = &u->options;
      break;
    case CURLUPART_HOST:
      storep = &u->host;
      break;
    case CURLUPART_ZONEID:
      storep = &u->zoneid;
      break;
    case CURLUPART_PORT:
      u->portnum = 0;
      storep = &u->port;
      break;
    case CURLUPART_PATH:
      storep = &u->path;
      break;
    case CURLUPART_QUERY:
      storep = &u->query;
      break;
    case CURLUPART_FRAGMENT:
      storep = &u->fragment;
      break;
    default:
      return CURLUE_UNKNOWN_PART;
    }
    if(storep && *storep) {
      Curl_safefree(*storep);
    }
    return CURLUE_OK;
  }

  switch(what) {
  case CURLUPART_SCHEME:
    if(strlen(part) > MAX_SCHEME_LEN)
      /* too long */
      return CURLUE_MALFORMED_INPUT;
    if(!(flags & CURLU_NON_SUPPORT_SCHEME) &&
       /* verify that it is a fine scheme */
       !Curl_builtin_scheme(part))
      return CURLUE_UNSUPPORTED_SCHEME;
    storep = &u->scheme;
    urlencode = FALSE; /* never */
    break;
  case CURLUPART_USER:
    storep = &u->user;
    break;
  case CURLUPART_PASSWORD:
    storep = &u->password;
    break;
  case CURLUPART_OPTIONS:
    storep = &u->options;
    break;
  case CURLUPART_HOST:
    storep = &u->host;
    Curl_safefree(u->zoneid);
    break;
  case CURLUPART_ZONEID:
    storep = &u->zoneid;
    break;
  case CURLUPART_PORT:
  {
    char *endp;
    urlencode = FALSE; /* never */
    port = strtol(part, &endp, 10);  /* Port number must be decimal */
    if((port <= 0) || (port > 0xffff))
      return CURLUE_BAD_PORT_NUMBER;
    if(*endp)
      /* weirdly provided number, not good! */
      return CURLUE_MALFORMED_INPUT;
    storep = &u->port;
  }
  break;
  case CURLUPART_PATH:
    urlskipslash = TRUE;
    storep = &u->path;
    break;
  case CURLUPART_QUERY:
    plusencode = urlencode;
    appendquery = (flags & CURLU_APPENDQUERY)?1:0;
    equalsencode = appendquery;
    storep = &u->query;
    break;
  case CURLUPART_FRAGMENT:
    storep = &u->fragment;
    break;
  case CURLUPART_URL: {
    /*
     * Allow a new URL to replace the existing (if any) contents.
     *
     * If the existing contents is enough for a URL, allow a relative URL to
     * replace it.
     */
    CURLUcode result;
    char *oldurl;
    char *redired_url;
    CURLU *handle2;

    if(Curl_is_absolute_url(part, NULL, MAX_SCHEME_LEN + 1)) {
      handle2 = curl_url();
      if(!handle2)
        return CURLUE_OUT_OF_MEMORY;
      result = parseurl(part, handle2, flags);
      if(!result)
        mv_urlhandle(handle2, u);
      else
        curl_url_cleanup(handle2);
      return result;
    }
    /* extract the full "old" URL to do the redirect on */
    result = curl_url_get(u, CURLUPART_URL, &oldurl, flags);
    if(result) {
      /* couldn't get the old URL, just use the new! */
      handle2 = curl_url();
      if(!handle2)
        return CURLUE_OUT_OF_MEMORY;
      result = parseurl(part, handle2, flags);
      if(!result)
        mv_urlhandle(handle2, u);
      else
        curl_url_cleanup(handle2);
      return result;
    }

    /* apply the relative part to create a new URL */
    redired_url = concat_url(oldurl, part);
    free(oldurl);
    if(!redired_url)
      return CURLUE_OUT_OF_MEMORY;

    /* now parse the new URL */
    handle2 = curl_url();
    if(!handle2) {
      free(redired_url);
      return CURLUE_OUT_OF_MEMORY;
    }
    result = parseurl(redired_url, handle2, flags);
    free(redired_url);
    if(!result)
      mv_urlhandle(handle2, u);
    else
      curl_url_cleanup(handle2);
    return result;
  }
  default:
    return CURLUE_UNKNOWN_PART;
  }
  DEBUGASSERT(storep);
  {
    const char *newp = part;
    size_t nalloc = strlen(part);

    if(nalloc > CURL_MAX_INPUT_LENGTH)
      /* excessive input length */
      return CURLUE_MALFORMED_INPUT;

    if(urlencode) {
      const unsigned char *i;
      char *o;
      char *enc = malloc(nalloc * 3 + 1); /* for worst case! */
      if(!enc)
        return CURLUE_OUT_OF_MEMORY;
      for(i = (const unsigned char *)part, o = enc; *i; i++) {
        if((*i == ' ') && plusencode) {
          *o = '+';
          o++;
        }
        else if(Curl_isunreserved(*i) ||
                ((*i == '/') && urlskipslash) ||
                ((*i == '=') && equalsencode)) {
          if((*i == '=') && equalsencode)
            /* only skip the first equals sign */
            equalsencode = FALSE;
          *o = *i;
          o++;
        }
        else {
          msnprintf(o, 4, "%%%02x", *i);
          o += 3;
        }
      }
      *o = 0; /* null-terminate */
      newp = enc;
    }
    else {
      char *p;
      newp = strdup(part);
      if(!newp)
        return CURLUE_OUT_OF_MEMORY;
      p = (char *)newp;
      while(*p) {
        /* make sure percent encoded are lower case */
        if((*p == '%') && ISXDIGIT(p[1]) && ISXDIGIT(p[2]) &&
           (ISUPPER(p[1]) || ISUPPER(p[2]))) {
          p[1] = (char)TOLOWER(p[1]);
          p[2] = (char)TOLOWER(p[2]);
          p += 3;
        }
        else
          p++;
      }
    }


// Source: urlapi.c
// Lines 1303-1530
