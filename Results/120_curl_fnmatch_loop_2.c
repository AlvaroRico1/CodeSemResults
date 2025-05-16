static int loop(const unsigned char *pattern, const unsigned char *string,
                int maxstars)
{
  unsigned char *p = (unsigned char *)pattern;
  unsigned char *s = (unsigned char *)string;
  unsigned char charset[CURLFNM_CHSET_SIZE] = { 0 };

  for(;;) {
    unsigned char *pp;

    switch(*p) {
    case '*':
      if(!maxstars)
        return CURL_FNMATCH_NOMATCH;
      /* Regroup consecutive stars and question marks. This can be done because
         '*?*?*' can be expressed as '??*'. */
      for(;;) {
        if(*++p == '\0')
          return CURL_FNMATCH_MATCH;
        if(*p == '?') {
          if(!*s++)
            return CURL_FNMATCH_NOMATCH;
        }
        else if(*p != '*')
          break;
      }
      /* Skip string characters until we find a match with pattern suffix. */
      for(maxstars--; *s; s++) {
        if(loop(p, s, maxstars) == CURL_FNMATCH_MATCH)
          return CURL_FNMATCH_MATCH;
      }
      return CURL_FNMATCH_NOMATCH;
    case '?':
      if(!*s)
        return CURL_FNMATCH_NOMATCH;
      s++;
      p++;
      break;
    case '\0':
      return *s? CURL_FNMATCH_NOMATCH: CURL_FNMATCH_MATCH;
    case '\\':
      if(p[1])
        p++;
      if(*s++ != *p++)
        return CURL_FNMATCH_NOMATCH;
      break;
    case '[':
      pp = p + 1; /* Copy in case of syntax error in set. */
      if(setcharset(&pp, charset)) {
        int found = FALSE;
        if(!*s)
          return CURL_FNMATCH_NOMATCH;
        if(charset[(unsigned int)*s])
          found = TRUE;
        else if(charset[CURLFNM_ALNUM])
          found = ISALNUM(*s);
        else if(charset[CURLFNM_ALPHA])
          found = ISALPHA(*s);
        else if(charset[CURLFNM_DIGIT])
          found = ISDIGIT(*s);
        else if(charset[CURLFNM_XDIGIT])
          found = ISXDIGIT(*s);
        else if(charset[CURLFNM_PRINT])
          found = ISPRINT(*s);
        else if(charset[CURLFNM_SPACE])
          found = ISSPACE(*s);
        else if(charset[CURLFNM_UPPER])
          found = ISUPPER(*s);
        else if(charset[CURLFNM_LOWER])
          found = ISLOWER(*s);
        else if(charset[CURLFNM_BLANK])
          found = ISBLANK(*s);
        else if(charset[CURLFNM_GRAPH])
          found = ISGRAPH(*s);

        if(charset[CURLFNM_NEGATE])
          found = !found;

        if(!found)
          return CURL_FNMATCH_NOMATCH;
        p = pp + 1;
        s++;
        break;
      }
      /* Syntax error in set; mismatch! */
      return CURL_FNMATCH_NOMATCH;

    default:
      if(*p++ != *s++)
        return CURL_FNMATCH_NOMATCH;
      break;
    }
  }


// Source: curl_fnmatch.c
// Lines 255-347
