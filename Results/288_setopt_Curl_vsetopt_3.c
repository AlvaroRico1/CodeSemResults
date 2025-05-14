// Source: curl/lib/setopt.c
// Lines 743-1934
CURLcode Curl_vsetopt(struct Curl_easy *data, CURLoption option, va_list param)
{
  char *argptr;
  CURLcode result = CURLE_OK;
  long arg;
  unsigned long uarg;
  curl_off_t bigsize;

  switch(option) {
  case CURLOPT_DNS_CACHE_TIMEOUT:
    arg = va_arg(param, long);
    if(arg < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.dns_cache_timeout = arg;
    break;
  case CURLOPT_DNS_USE_GLOBAL_CACHE:
    /* deprecated */
    break;
  case CURLOPT_SSL_CIPHER_LIST:
    /* set a list of cipher we want to use in the SSL connection */
    result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER_LIST],
                            va_arg(param, char *));
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSL_CIPHER_LIST:
    /* set a list of cipher we want to use in the SSL connection for proxy */
    result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER_LIST_PROXY],
                            va_arg(param, char *));
    break;
#endif
  case CURLOPT_TLS13_CIPHERS:
    if(Curl_ssl_tls13_ciphersuites()) {
      /* set preferred list of TLS 1.3 cipher suites */
      result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER13_LIST],
                              va_arg(param, char *));
    }
    else
      return CURLE_NOT_BUILT_IN;
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_TLS13_CIPHERS:
    if(Curl_ssl_tls13_ciphersuites()) {
      /* set preferred list of TLS 1.3 cipher suites for proxy */
      result = Curl_setstropt(&data->set.str[STRING_SSL_CIPHER13_LIST_PROXY],
                              va_arg(param, char *));
    }
    else
      return CURLE_NOT_BUILT_IN;
    break;
#endif
  case CURLOPT_RANDOM_FILE:
    /*
     * This is the path name to a file that contains random data to seed
     * the random SSL stuff with. The file is only used for reading.
     */
    result = Curl_setstropt(&data->set.str[STRING_SSL_RANDOM_FILE],
                            va_arg(param, char *));
    break;
  case CURLOPT_EGDSOCKET:
    /*
     * The Entropy Gathering Daemon socket pathname
     */
    result = Curl_setstropt(&data->set.str[STRING_SSL_EGDSOCKET],
                            va_arg(param, char *));
    break;
  case CURLOPT_MAXCONNECTS:
    /*
     * Set the absolute number of maximum simultaneous alive connection that
     * libcurl is allowed to have.
     */
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.maxconnects = arg;
    break;
  case CURLOPT_FORBID_REUSE:
    /*
     * When this transfer is done, it must not be left to be reused by a
     * subsequent transfer but shall be closed immediately.
     */
    data->set.reuse_forbid = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_FRESH_CONNECT:
    /*
     * This transfer shall not use a previously cached connection but
     * should be made with a fresh new connect!
     */
    data->set.reuse_fresh = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_VERBOSE:
    /*
     * Verbose means infof() calls that give a lot of information about
     * the connection and transfer procedures as well as internal choices.
     */
    data->set.verbose = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_HEADER:
    /*
     * Set to include the header in the general data output stream.
     */
    data->set.include_header = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_NOPROGRESS:
    /*
     * Shut off the internal supported progress meter
     */
    data->set.hide_progress = (0 != va_arg(param, long)) ? TRUE : FALSE;
    if(data->set.hide_progress)
      data->progress.flags |= PGRS_HIDE;
    else
      data->progress.flags &= ~PGRS_HIDE;
    break;
  case CURLOPT_NOBODY:
    /*
     * Do not include the body part in the output data stream.
     */
    data->set.opt_no_body = (0 != va_arg(param, long)) ? TRUE : FALSE;
#ifndef CURL_DISABLE_HTTP
    if(data->set.opt_no_body)
      /* in HTTP lingo, no body means using the HEAD request... */
      data->set.method = HTTPREQ_HEAD;
    else if(data->set.method == HTTPREQ_HEAD)
      data->set.method = HTTPREQ_GET;
#endif
    break;
  case CURLOPT_FAILONERROR:
    /*
     * Don't output the >=400 error code HTML-page, but instead only
     * return error.
     */
    data->set.http_fail_on_error = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_KEEP_SENDING_ON_ERROR:
    data->set.http_keep_sending_on_error = (0 != va_arg(param, long)) ?
      TRUE : FALSE;
    break;
  case CURLOPT_UPLOAD:
  case CURLOPT_PUT:
    /*
     * We want to sent data to the remote host. If this is HTTP, that equals
     * using the PUT request.
     */
    data->set.upload = (0 != va_arg(param, long)) ? TRUE : FALSE;
    if(data->set.upload) {
      /* If this is HTTP, PUT is what's needed to "upload" */
      data->set.method = HTTPREQ_PUT;
      data->set.opt_no_body = FALSE; /* this is implied */
    }
    else
      /* In HTTP, the opposite of upload is GET (unless NOBODY is true as
         then this can be changed to HEAD later on) */
      data->set.method = HTTPREQ_GET;
    break;
  case CURLOPT_REQUEST_TARGET:
    result = Curl_setstropt(&data->set.str[STRING_TARGET],
                            va_arg(param, char *));
    break;
  case CURLOPT_FILETIME:
    /*
     * Try to get the file time of the remote document. The time will
     * later (possibly) become available using curl_easy_getinfo().
     */
    data->set.get_filetime = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_SERVER_RESPONSE_TIMEOUT:
    /*
     * Option that specifies how quickly an server response must be obtained
     * before it is considered failure. For pingpong protocols.
     */
    arg = va_arg(param, long);
    if((arg >= 0) && (arg <= (INT_MAX/1000)))
      data->set.server_response_timeout = arg * 1000;
    else
      return CURLE_BAD_FUNCTION_ARGUMENT;
    break;
#ifndef CURL_DISABLE_TFTP
  case CURLOPT_TFTP_NO_OPTIONS:
    /*
     * Option that prevents libcurl from sending TFTP option requests to the
     * server.
     */
    data->set.tftp_no_options = va_arg(param, long) != 0;
    break;
  case CURLOPT_TFTP_BLKSIZE:
    /*
     * TFTP option that specifies the block size to use for data transmission.
     */
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.tftp_blksize = arg;
    break;
#endif
#ifndef CURL_DISABLE_NETRC
  case CURLOPT_NETRC:
    /*
     * Parse the $HOME/.netrc file
     */
    arg = va_arg(param, long);
    if((arg < CURL_NETRC_IGNORED) || (arg >= CURL_NETRC_LAST))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.use_netrc = (enum CURL_NETRC_OPTION)arg;
    break;
  case CURLOPT_NETRC_FILE:
    /*
     * Use this file instead of the $HOME/.netrc file
     */
    result = Curl_setstropt(&data->set.str[STRING_NETRC_FILE],
                            va_arg(param, char *));
    break;
#endif
  case CURLOPT_TRANSFERTEXT:
    /*
     * This option was previously named 'FTPASCII'. Renamed to work with
     * more protocols than merely FTP.
     *
     * Transfer using ASCII (instead of BINARY).
     */
    data->set.prefer_ascii = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
  case CURLOPT_TIMECONDITION:
    /*
     * Set HTTP time condition. This must be one of the defines in the
     * curl/curl.h header file.
     */
    arg = va_arg(param, long);
    if((arg < CURL_TIMECOND_NONE) || (arg >= CURL_TIMECOND_LAST))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.timecondition = (curl_TimeCond)arg;
    break;
  case CURLOPT_TIMEVALUE:
    /*
     * This is the value to compare with the remote document with the
     * method set with CURLOPT_TIMECONDITION
     */
    data->set.timevalue = (time_t)va_arg(param, long);
    break;

  case CURLOPT_TIMEVALUE_LARGE:
    /*
     * This is the value to compare with the remote document with the
     * method set with CURLOPT_TIMECONDITION
     */
    data->set.timevalue = (time_t)va_arg(param, curl_off_t);
    break;

  case CURLOPT_SSLVERSION:
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSLVERSION:
#endif
    /*
     * Set explicit SSL version to try to connect with, as some SSL
     * implementations are lame.
     */
#ifdef USE_SSL
    {
      long version, version_max;
      struct ssl_primary_config *primary = &data->set.ssl.primary;
#ifndef CURL_DISABLE_PROXY
      if(option != CURLOPT_SSLVERSION)
        primary = &data->set.proxy_ssl.primary;
#endif

      arg = va_arg(param, long);

      version = C_SSLVERSION_VALUE(arg);
      version_max = C_SSLVERSION_MAX_VALUE(arg);

      if(version < CURL_SSLVERSION_DEFAULT ||
         version == CURL_SSLVERSION_SSLv2 ||
         version == CURL_SSLVERSION_SSLv3 ||
         version >= CURL_SSLVERSION_LAST ||
         version_max < CURL_SSLVERSION_MAX_NONE ||
         version_max >= CURL_SSLVERSION_MAX_LAST)
        return CURLE_BAD_FUNCTION_ARGUMENT;

      primary->version = version;
      primary->version_max = version_max;
    }
#else
    result = CURLE_NOT_BUILT_IN;
#endif
    break;

    /* MQTT "borrows" some of the HTTP options */
#if !defined(CURL_DISABLE_HTTP) || !defined(CURL_DISABLE_MQTT)
  case CURLOPT_COPYPOSTFIELDS:
    /*
     * A string with POST data. Makes curl HTTP POST. Even if it is NULL.
     * If needed, CURLOPT_POSTFIELDSIZE must have been set prior to
     *  CURLOPT_COPYPOSTFIELDS and not altered later.
     */
    argptr = va_arg(param, char *);

    if(!argptr || data->set.postfieldsize == -1)
      result = Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], argptr);
    else {
      /*
       *  Check that requested length does not overflow the size_t type.
       */

      if((data->set.postfieldsize < 0) ||
         ((sizeof(curl_off_t) != sizeof(size_t)) &&
          (data->set.postfieldsize > (curl_off_t)((size_t)-1))))
        result = CURLE_OUT_OF_MEMORY;
      else {
        char *p;

        (void) Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);

        /* Allocate even when size == 0. This satisfies the need of possible
           later address compare to detect the COPYPOSTFIELDS mode, and
           to mark that postfields is used rather than read function or
           form data.
        */
        p = malloc((size_t)(data->set.postfieldsize?
                            data->set.postfieldsize:1));

        if(!p)
          result = CURLE_OUT_OF_MEMORY;
        else {
          if(data->set.postfieldsize)
            memcpy(p, argptr, (size_t)data->set.postfieldsize);

          data->set.str[STRING_COPYPOSTFIELDS] = p;
        }
      }
    }

    data->set.postfields = data->set.str[STRING_COPYPOSTFIELDS];
    data->set.method = HTTPREQ_POST;
    break;

  case CURLOPT_POSTFIELDS:
    /*
     * Like above, but use static data instead of copying it.
     */
    data->set.postfields = va_arg(param, void *);
    /* Release old copied data. */
    (void) Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
    data->set.method = HTTPREQ_POST;
    break;

  case CURLOPT_POSTFIELDSIZE:
    /*
     * The size of the POSTFIELD data to prevent libcurl to do strlen() to
     * figure it out. Enables binary posts.
     */
    bigsize = va_arg(param, long);
    if(bigsize < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;

    if(data->set.postfieldsize < bigsize &&
       data->set.postfields == data->set.str[STRING_COPYPOSTFIELDS]) {
      /* Previous CURLOPT_COPYPOSTFIELDS is no longer valid. */
      (void) Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
      data->set.postfields = NULL;
    }

    data->set.postfieldsize = bigsize;
    break;

  case CURLOPT_POSTFIELDSIZE_LARGE:
    /*
     * The size of the POSTFIELD data to prevent libcurl to do strlen() to
     * figure it out. Enables binary posts.
     */
    bigsize = va_arg(param, curl_off_t);
    if(bigsize < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;

    if(data->set.postfieldsize < bigsize &&
       data->set.postfields == data->set.str[STRING_COPYPOSTFIELDS]) {
      /* Previous CURLOPT_COPYPOSTFIELDS is no longer valid. */
      (void) Curl_setstropt(&data->set.str[STRING_COPYPOSTFIELDS], NULL);
      data->set.postfields = NULL;
    }

    data->set.postfieldsize = bigsize;
    break;
#endif
#ifndef CURL_DISABLE_HTTP
  case CURLOPT_AUTOREFERER:
    /*
     * Switch on automatic referer that gets set if curl follows locations.
     */
    data->set.http_auto_referer = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_ACCEPT_ENCODING:
    /*
     * String to use at the value of Accept-Encoding header.
     *
     * If the encoding is set to "" we use an Accept-Encoding header that
     * encompasses all the encodings we support.
     * If the encoding is set to NULL we don't send an Accept-Encoding header
     * and ignore an received Content-Encoding header.
     *
     */
    argptr = va_arg(param, char *);
    if(argptr && !*argptr) {
      argptr = Curl_all_content_encodings();
      if(!argptr)
        result = CURLE_OUT_OF_MEMORY;
      else {
        result = Curl_setstropt(&data->set.str[STRING_ENCODING], argptr);
        free(argptr);
      }
    }
    else
      result = Curl_setstropt(&data->set.str[STRING_ENCODING], argptr);
    break;

  case CURLOPT_TRANSFER_ENCODING:
    data->set.http_transfer_encoding = (0 != va_arg(param, long)) ?
      TRUE : FALSE;
    break;

  case CURLOPT_FOLLOWLOCATION:
    /*
     * Follow Location: header hints on a HTTP-server.
     */
    data->set.http_follow_location = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_UNRESTRICTED_AUTH:
    /*
     * Send authentication (user+password) when following locations, even when
     * hostname changed.
     */
    data->set.allow_auth_to_other_hosts =
      (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_MAXREDIRS:
    /*
     * The maximum amount of hops you allow curl to follow Location:
     * headers. This should mostly be used to detect never-ending loops.
     */
    arg = va_arg(param, long);
    if(arg < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.maxredirs = arg;
    break;

  case CURLOPT_POSTREDIR:
    /*
     * Set the behavior of POST when redirecting
     * CURL_REDIR_GET_ALL - POST is changed to GET after 301 and 302
     * CURL_REDIR_POST_301 - POST is kept as POST after 301
     * CURL_REDIR_POST_302 - POST is kept as POST after 302
     * CURL_REDIR_POST_303 - POST is kept as POST after 303
     * CURL_REDIR_POST_ALL - POST is kept as POST after 301, 302 and 303
     * other - POST is kept as POST after 301 and 302
     */
    arg = va_arg(param, long);
    if(arg < CURL_REDIR_GET_ALL)
      /* no return error on too high numbers since the bitmask could be
         extended in a future */
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.keep_post = arg & CURL_REDIR_POST_ALL;
    break;

  case CURLOPT_POST:
    /* Does this option serve a purpose anymore? Yes it does, when
       CURLOPT_POSTFIELDS isn't used and the POST data is read off the
       callback! */
    if(va_arg(param, long)) {
      data->set.method = HTTPREQ_POST;
      data->set.opt_no_body = FALSE; /* this is implied */
    }
    else
      data->set.method = HTTPREQ_GET;
    break;

  case CURLOPT_HTTPPOST:
    /*
     * Set to make us do HTTP POST
     */
    data->set.httppost = va_arg(param, struct curl_httppost *);
    data->set.method = HTTPREQ_POST_FORM;
    data->set.opt_no_body = FALSE; /* this is implied */
    break;

  case CURLOPT_AWS_SIGV4:
    /*
     * String that is merged to some authentication
     * parameters are used by the algorithm.
     */
    result = Curl_setstropt(&data->set.str[STRING_AWS_SIGV4],
                            va_arg(param, char *));
    /*
     * Basic been set by default it need to be unset here
     */
    if(data->set.str[STRING_AWS_SIGV4])
      data->set.httpauth = CURLAUTH_AWS_SIGV4;
    break;

  case CURLOPT_MIMEPOST:
    /*
     * Set to make us do MIME/form POST
     */
    result = Curl_mime_set_subparts(&data->set.mimepost,
                                    va_arg(param, curl_mime *), FALSE);
    if(!result) {
      data->set.method = HTTPREQ_POST_MIME;
      data->set.opt_no_body = FALSE; /* this is implied */
    }
    break;

  case CURLOPT_REFERER:
    /*
     * String to set in the HTTP Referer: field.
     */
    if(data->state.referer_alloc) {
      Curl_safefree(data->state.referer);
      data->state.referer_alloc = FALSE;
    }
    result = Curl_setstropt(&data->set.str[STRING_SET_REFERER],
                            va_arg(param, char *));
    data->state.referer = data->set.str[STRING_SET_REFERER];
    break;

  case CURLOPT_USERAGENT:
    /*
     * String to use in the HTTP User-Agent field
     */
    result = Curl_setstropt(&data->set.str[STRING_USERAGENT],
                            va_arg(param, char *));
    break;

  case CURLOPT_HTTPHEADER:
    /*
     * Set a list with HTTP headers to use (or replace internals with)
     */
    data->set.headers = va_arg(param, struct curl_slist *);
    break;

#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXYHEADER:
    /*
     * Set a list with proxy headers to use (or replace internals with)
     *
     * Since CURLOPT_HTTPHEADER was the only way to set HTTP headers for a
     * long time we remain doing it this way until CURLOPT_PROXYHEADER is
     * used. As soon as this option has been used, if set to anything but
     * NULL, custom headers for proxies are only picked from this list.
     *
     * Set this option to NULL to restore the previous behavior.
     */
    data->set.proxyheaders = va_arg(param, struct curl_slist *);
    break;
#endif
  case CURLOPT_HEADEROPT:
    /*
     * Set header option.
     */
    arg = va_arg(param, long);
    data->set.sep_headers = (bool)((arg & CURLHEADER_SEPARATE)? TRUE: FALSE);
    break;

  case CURLOPT_HTTP200ALIASES:
    /*
     * Set a list of aliases for HTTP 200 in response header
     */
    data->set.http200aliases = va_arg(param, struct curl_slist *);
    break;

#if !defined(CURL_DISABLE_COOKIES)
  case CURLOPT_COOKIE:
    /*
     * Cookie string to send to the remote server in the request.
     */
    result = Curl_setstropt(&data->set.str[STRING_COOKIE],
                            va_arg(param, char *));
    break;

  case CURLOPT_COOKIEFILE:
    /*
     * Set cookie file to read and parse. Can be used multiple times.
     */
    argptr = (char *)va_arg(param, void *);
    if(argptr) {
      struct curl_slist *cl;
      /* general protection against mistakes and abuse */
      if(strlen(argptr) > CURL_MAX_INPUT_LENGTH)
        return CURLE_BAD_FUNCTION_ARGUMENT;
      /* append the cookie file name to the list of file names, and deal with
         them later */
      cl = curl_slist_append(data->state.cookielist, argptr);
      if(!cl) {
        curl_slist_free_all(data->state.cookielist);
        data->state.cookielist = NULL;
        return CURLE_OUT_OF_MEMORY;
      }
      data->state.cookielist = cl; /* store the list for later use */
    }
    else {
      /* clear the list of cookie files */
      curl_slist_free_all(data->state.cookielist);
      data->state.cookielist = NULL;

      if(!data->share || !data->share->cookies) {
        /* throw away all existing cookies if this isn't a shared cookie
           container */
        Curl_cookie_clearall(data->cookies);
        Curl_cookie_cleanup(data->cookies);
      }
      /* disable the cookie engine */
      data->cookies = NULL;
    }
    break;

  case CURLOPT_COOKIEJAR:
    /*
     * Set cookie file name to dump all cookies to when we're done.
     */
  {
    struct CookieInfo *newcookies;
    result = Curl_setstropt(&data->set.str[STRING_COOKIEJAR],
                            va_arg(param, char *));

    /*
     * Activate the cookie parser. This may or may not already
     * have been made.
     */
    newcookies = Curl_cookie_init(data, NULL, data->cookies,
                                  data->set.cookiesession);
    if(!newcookies)
      result = CURLE_OUT_OF_MEMORY;
    data->cookies = newcookies;
  }
  break;

  case CURLOPT_COOKIESESSION:
    /*
     * Set this option to TRUE to start a new "cookie session". It will
     * prevent the forthcoming read-cookies-from-file actions to accept
     * cookies that are marked as being session cookies, as they belong to a
     * previous session.
     *
     * In the original Netscape cookie spec, "session cookies" are cookies
     * with no expire date set. RFC2109 describes the same action if no
     * 'Max-Age' is set and RFC2965 includes the RFC2109 description and adds
     * a 'Discard' action that can enforce the discard even for cookies that
     * have a Max-Age.
     *
     * We run mostly with the original cookie spec, as hardly anyone implements
     * anything else.
     */
    data->set.cookiesession = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_COOKIELIST:
    argptr = va_arg(param, char *);

    if(!argptr)
      break;

    if(strcasecompare(argptr, "ALL")) {
      /* clear all cookies */
      Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
      Curl_cookie_clearall(data->cookies);
      Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
    }
    else if(strcasecompare(argptr, "SESS")) {
      /* clear session cookies */
      Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
      Curl_cookie_clearsess(data->cookies);
      Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
    }
    else if(strcasecompare(argptr, "FLUSH")) {
      /* flush cookies to file, takes care of the locking */
      Curl_flush_cookies(data, FALSE);
    }
    else if(strcasecompare(argptr, "RELOAD")) {
      /* reload cookies from file */
      Curl_cookie_loadfiles(data);
      break;
    }
    else {
      if(!data->cookies)
        /* if cookie engine was not running, activate it */
        data->cookies = Curl_cookie_init(data, NULL, NULL, TRUE);

      /* general protection against mistakes and abuse */
      if(strlen(argptr) > CURL_MAX_INPUT_LENGTH)
        return CURLE_BAD_FUNCTION_ARGUMENT;
      argptr = strdup(argptr);
      if(!argptr || !data->cookies) {
        result = CURLE_OUT_OF_MEMORY;
        free(argptr);
      }
      else {
        Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);

        if(checkprefix("Set-Cookie:", argptr))
          /* HTTP Header format line */
          Curl_cookie_add(data, data->cookies, TRUE, FALSE, argptr + 11, NULL,
                          NULL, TRUE);

        else
          /* Netscape format line */
          Curl_cookie_add(data, data->cookies, FALSE, FALSE, argptr, NULL,
                          NULL, TRUE);

        Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
        free(argptr);
      }
    }

    break;
#endif /* !CURL_DISABLE_COOKIES */

  case CURLOPT_HTTPGET:
    /*
     * Set to force us do HTTP GET
     */
    if(va_arg(param, long)) {
      data->set.method = HTTPREQ_GET;
      data->set.upload = FALSE; /* switch off upload */
      data->set.opt_no_body = FALSE; /* this is implied */
    }
    break;

  case CURLOPT_HTTP_VERSION:
    /*
     * This sets a requested HTTP version to be used. The value is one of
     * the listed enums in curl/curl.h.
     */
    arg = va_arg(param, long);
    if(arg < CURL_HTTP_VERSION_NONE)
      return CURLE_BAD_FUNCTION_ARGUMENT;
#ifdef ENABLE_QUIC
    if(arg == CURL_HTTP_VERSION_3)
      ;
    else
#endif
#if !defined(USE_NGHTTP2) && !defined(USE_HYPER)
    if(arg >= CURL_HTTP_VERSION_2)
      return CURLE_UNSUPPORTED_PROTOCOL;
#else
    if(arg >= CURL_HTTP_VERSION_LAST)
      return CURLE_UNSUPPORTED_PROTOCOL;
    if(arg == CURL_HTTP_VERSION_NONE)
      arg = CURL_HTTP_VERSION_2TLS;
#endif
    data->set.httpwant = (unsigned char)arg;
    break;

  case CURLOPT_EXPECT_100_TIMEOUT_MS:
    /*
     * Time to wait for a response to a HTTP request containing an
     * Expect: 100-continue header before sending the data anyway.
     */
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.expect_100_timeout = arg;
    break;

  case CURLOPT_HTTP09_ALLOWED:
    arg = va_arg(param, unsigned long);
    if(arg > 1L)
      return CURLE_BAD_FUNCTION_ARGUMENT;
#ifdef USE_HYPER
    /* Hyper does not support HTTP/0.9 */
    if(arg)
      return CURLE_BAD_FUNCTION_ARGUMENT;
#else
    data->set.http09_allowed = arg ? TRUE : FALSE;
#endif
    break;
#endif   /* CURL_DISABLE_HTTP */

  case CURLOPT_HTTPAUTH:
    /*
     * Set HTTP Authentication type BITMASK.
     */
  {
    int bitcheck;
    bool authbits;
    unsigned long auth = va_arg(param, unsigned long);

    if(auth == CURLAUTH_NONE) {
      data->set.httpauth = auth;
      break;
    }

    /* the DIGEST_IE bit is only used to set a special marker, for all the
       rest we need to handle it as normal DIGEST */
    data->state.authhost.iestyle =
      (bool)((auth & CURLAUTH_DIGEST_IE) ? TRUE : FALSE);

    if(auth & CURLAUTH_DIGEST_IE) {
      auth |= CURLAUTH_DIGEST; /* set standard digest bit */
      auth &= ~CURLAUTH_DIGEST_IE; /* unset ie digest bit */
    }

    /* switch off bits we can't support */
#ifndef USE_NTLM
    auth &= ~CURLAUTH_NTLM;    /* no NTLM support */
    auth &= ~CURLAUTH_NTLM_WB; /* no NTLM_WB support */
#elif !defined(NTLM_WB_ENABLED)
    auth &= ~CURLAUTH_NTLM_WB; /* no NTLM_WB support */
#endif
#ifndef USE_SPNEGO
    auth &= ~CURLAUTH_NEGOTIATE; /* no Negotiate (SPNEGO) auth without
                                    GSS-API or SSPI */
#endif

    /* check if any auth bit lower than CURLAUTH_ONLY is still set */
    bitcheck = 0;
    authbits = FALSE;
    while(bitcheck < 31) {
      if(auth & (1UL << bitcheck++)) {
        authbits = TRUE;
        break;
      }
    }
    if(!authbits)
      return CURLE_NOT_BUILT_IN; /* no supported types left! */

    data->set.httpauth = auth;
  }
  break;

  case CURLOPT_CUSTOMREQUEST:
    /*
     * Set a custom string to use as request
     */
    result = Curl_setstropt(&data->set.str[STRING_CUSTOMREQUEST],
                            va_arg(param, char *));

    /* we don't set
       data->set.method = HTTPREQ_CUSTOM;
       here, we continue as if we were using the already set type
       and this just changes the actual request keyword */
    break;

#ifndef CURL_DISABLE_PROXY
  case CURLOPT_HTTPPROXYTUNNEL:
    /*
     * Tunnel operations through the proxy instead of normal proxy use
     */
    data->set.tunnel_thru_httpproxy = (0 != va_arg(param, long)) ?
      TRUE : FALSE;
    break;

  case CURLOPT_PROXYPORT:
    /*
     * Explicitly set HTTP proxy port number.
     */
    arg = va_arg(param, long);
    if((arg < 0) || (arg > 65535))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.proxyport = arg;
    break;

  case CURLOPT_PROXYAUTH:
    /*
     * Set HTTP Authentication type BITMASK.
     */
  {
    int bitcheck;
    bool authbits;
    unsigned long auth = va_arg(param, unsigned long);

    if(auth == CURLAUTH_NONE) {
      data->set.proxyauth = auth;
      break;
    }

    /* the DIGEST_IE bit is only used to set a special marker, for all the
       rest we need to handle it as normal DIGEST */
    data->state.authproxy.iestyle =
      (bool)((auth & CURLAUTH_DIGEST_IE) ? TRUE : FALSE);

    if(auth & CURLAUTH_DIGEST_IE) {
      auth |= CURLAUTH_DIGEST; /* set standard digest bit */
      auth &= ~CURLAUTH_DIGEST_IE; /* unset ie digest bit */
    }
    /* switch off bits we can't support */
#ifndef USE_NTLM
    auth &= ~CURLAUTH_NTLM;    /* no NTLM support */
    auth &= ~CURLAUTH_NTLM_WB; /* no NTLM_WB support */
#elif !defined(NTLM_WB_ENABLED)
    auth &= ~CURLAUTH_NTLM_WB; /* no NTLM_WB support */
#endif
#ifndef USE_SPNEGO
    auth &= ~CURLAUTH_NEGOTIATE; /* no Negotiate (SPNEGO) auth without
                                    GSS-API or SSPI */
#endif

    /* check if any auth bit lower than CURLAUTH_ONLY is still set */
    bitcheck = 0;
    authbits = FALSE;
    while(bitcheck < 31) {
      if(auth & (1UL << bitcheck++)) {
        authbits = TRUE;
        break;
      }
    }
    if(!authbits)
      return CURLE_NOT_BUILT_IN; /* no supported types left! */

    data->set.proxyauth = auth;
  }
  break;

  case CURLOPT_PROXY:
    /*
     * Set proxy server:port to use as proxy.
     *
     * If the proxy is set to "" (and CURLOPT_SOCKS_PROXY is set to "" or NULL)
     * we explicitly say that we don't want to use a proxy
     * (even though there might be environment variables saying so).
     *
     * Setting it to NULL, means no proxy but allows the environment variables
     * to decide for us (if CURLOPT_SOCKS_PROXY setting it to NULL).
     */
    result = Curl_setstropt(&data->set.str[STRING_PROXY],
                            va_arg(param, char *));
    break;

  case CURLOPT_PRE_PROXY:
    /*
     * Set proxy server:port to use as SOCKS proxy.
     *
     * If the proxy is set to "" or NULL we explicitly say that we don't want
     * to use the socks proxy.
     */
    result = Curl_setstropt(&data->set.str[STRING_PRE_PROXY],
                            va_arg(param, char *));
    break;

  case CURLOPT_PROXYTYPE:
    /*
     * Set proxy type. HTTP/HTTP_1_0/SOCKS4/SOCKS4a/SOCKS5/SOCKS5_HOSTNAME
     */
    arg = va_arg(param, long);
    if((arg < CURLPROXY_HTTP) || (arg > CURLPROXY_SOCKS5_HOSTNAME))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.proxytype = (curl_proxytype)arg;
    break;

  case CURLOPT_PROXY_TRANSFER_MODE:
    /*
     * set transfer mode (;type=<a|i>) when doing FTP via an HTTP proxy
     */
    switch(va_arg(param, long)) {
    case 0:
      data->set.proxy_transfer_mode = FALSE;
      break;
    case 1:
      data->set.proxy_transfer_mode = TRUE;
      break;
    default:
      /* reserve other values for future use */
      result = CURLE_BAD_FUNCTION_ARGUMENT;
      break;
    }
    break;
#endif   /* CURL_DISABLE_PROXY */

  case CURLOPT_SOCKS5_AUTH:
    data->set.socks5auth = va_arg(param, unsigned long);
    if(data->set.socks5auth & ~(CURLAUTH_BASIC | CURLAUTH_GSSAPI))
      result = CURLE_NOT_BUILT_IN;
    break;
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
  case CURLOPT_SOCKS5_GSSAPI_NEC:
    /*
     * Set flag for NEC SOCK5 support
     */
    data->set.socks5_gssapi_nec = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
#endif
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_SOCKS5_GSSAPI_SERVICE:
  case CURLOPT_PROXY_SERVICE_NAME:
    /*
     * Set proxy authentication service name for Kerberos 5 and SPNEGO
     */
    result = Curl_setstropt(&data->set.str[STRING_PROXY_SERVICE_NAME],
                            va_arg(param, char *));
    break;
#endif
  case CURLOPT_SERVICE_NAME:
    /*
     * Set authentication service name for DIGEST-MD5, Kerberos 5 and SPNEGO
     */
    result = Curl_setstropt(&data->set.str[STRING_SERVICE_NAME],
                            va_arg(param, char *));
    break;

  case CURLOPT_HEADERDATA:
    /*
     * Custom pointer to pass the header write callback function
     */
    data->set.writeheader = (void *)va_arg(param, void *);
    break;
  case CURLOPT_ERRORBUFFER:
    /*
     * Error buffer provided by the caller to get the human readable
     * error string in.
     */
    data->set.errorbuffer = va_arg(param, char *);
    break;
  case CURLOPT_WRITEDATA:
    /*
     * FILE pointer to write to. Or possibly
     * used as argument to the write callback.
     */
    data->set.out = va_arg(param, void *);
    break;

  case CURLOPT_DIRLISTONLY:
    /*
     * An option that changes the command to one that asks for a list only, no
     * file info details. Used for FTP, POP3 and SFTP.
     */
    data->set.list_only = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_APPEND:
    /*
     * We want to upload and append to an existing file. Used for FTP and
     * SFTP.
     */
    data->set.remote_append = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

#ifndef CURL_DISABLE_FTP
  case CURLOPT_FTP_FILEMETHOD:
    /*
     * How do access files over FTP.
     */
    arg = va_arg(param, long);
    if((arg < CURLFTPMETHOD_DEFAULT) || (arg >= CURLFTPMETHOD_LAST))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.ftp_filemethod = (curl_ftpfile)arg;
    break;
  case CURLOPT_FTPPORT:
    /*
     * Use FTP PORT, this also specifies which IP address to use
     */
    result = Curl_setstropt(&data->set.str[STRING_FTPPORT],
                            va_arg(param, char *));
    data->set.ftp_use_port = (data->set.str[STRING_FTPPORT]) ? TRUE : FALSE;
    break;

  case CURLOPT_FTP_USE_EPRT:
    data->set.ftp_use_eprt = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_FTP_USE_EPSV:
    data->set.ftp_use_epsv = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_FTP_USE_PRET:
    data->set.ftp_use_pret = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_FTP_SSL_CCC:
    arg = va_arg(param, long);
    if((arg < CURLFTPSSL_CCC_NONE) || (arg >= CURLFTPSSL_CCC_LAST))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.ftp_ccc = (curl_ftpccc)arg;
    break;

  case CURLOPT_FTP_SKIP_PASV_IP:
    /*
     * Enable or disable FTP_SKIP_PASV_IP, which will disable/enable the
     * bypass of the IP address in PASV responses.
     */
    data->set.ftp_skip_ip = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;

  case CURLOPT_FTP_ACCOUNT:
    result = Curl_setstropt(&data->set.str[STRING_FTP_ACCOUNT],
                            va_arg(param, char *));
    break;

  case CURLOPT_FTP_ALTERNATIVE_TO_USER:
    result = Curl_setstropt(&data->set.str[STRING_FTP_ALTERNATIVE_TO_USER],
                            va_arg(param, char *));
    break;

  case CURLOPT_FTPSSLAUTH:
    /*
     * Set a specific auth for FTP-SSL transfers.
     */
    arg = va_arg(param, long);
    if((arg < CURLFTPAUTH_DEFAULT) || (arg >= CURLFTPAUTH_LAST))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.ftpsslauth = (curl_ftpauth)arg;
    break;
  case CURLOPT_KRBLEVEL:
    /*
     * A string that defines the kerberos security level.
     */
    result = Curl_setstropt(&data->set.str[STRING_KRB_LEVEL],
                            va_arg(param, char *));
    data->set.krb = (data->set.str[STRING_KRB_LEVEL]) ? TRUE : FALSE;
    break;
#endif
  case CURLOPT_FTP_CREATE_MISSING_DIRS:
    /*
     * An FTP/SFTP option that modifies an upload to create missing
     * directories on the server.
     */
    arg = va_arg(param, long);
    /* reserve other values for future use */
    if((arg < CURLFTP_CREATE_DIR_NONE) ||
       (arg > CURLFTP_CREATE_DIR_RETRY))
      result = CURLE_BAD_FUNCTION_ARGUMENT;
    else
      data->set.ftp_create_missing_dirs = (int)arg;
    break;
  case CURLOPT_READDATA:
    /*
     * FILE pointer to read the file to be uploaded from. Or possibly
     * used as argument to the read callback.
     */
    data->set.in_set = va_arg(param, void *);
    break;
  case CURLOPT_INFILESIZE:
    /*
     * If known, this should inform curl about the file size of the
     * to-be-uploaded file.
     */
    arg = va_arg(param, long);
    if(arg < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.filesize = arg;
    break;
  case CURLOPT_INFILESIZE_LARGE:
    /*
     * If known, this should inform curl about the file size of the
     * to-be-uploaded file.
     */
    bigsize = va_arg(param, curl_off_t);
    if(bigsize < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.filesize = bigsize;
    break;
  case CURLOPT_LOW_SPEED_LIMIT:
    /*
     * The low speed limit that if transfers are below this for
     * CURLOPT_LOW_SPEED_TIME, the transfer is aborted.
     */
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.low_speed_limit = arg;
    break;
  case CURLOPT_MAX_SEND_SPEED_LARGE:
    /*
     * When transfer uploads are faster then CURLOPT_MAX_SEND_SPEED_LARGE
     * bytes per second the transfer is throttled..
     */
    bigsize = va_arg(param, curl_off_t);
    if(bigsize < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.max_send_speed = bigsize;
    break;
  case CURLOPT_MAX_RECV_SPEED_LARGE:
    /*
     * When receiving data faster than CURLOPT_MAX_RECV_SPEED_LARGE bytes per
     * second the transfer is throttled..
     */
    bigsize = va_arg(param, curl_off_t);
    if(bigsize < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.max_recv_speed = bigsize;
    break;
  case CURLOPT_LOW_SPEED_TIME:
    /*
     * The low speed time that if transfers are below the set
     * CURLOPT_LOW_SPEED_LIMIT during this time, the transfer is aborted.
     */
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.low_speed_time = arg;
    break;
  case CURLOPT_CURLU:
    /*
     * pass CURLU to set URL
     */
    data->set.uh = va_arg(param, CURLU *);
    break;
  case CURLOPT_URL:
    /*
     * The URL to fetch.
     */
    if(data->state.url_alloc) {
      /* the already set URL is allocated, free it first! */
      Curl_safefree(data->state.url);
      data->state.url_alloc = FALSE;
    }
    result = Curl_setstropt(&data->set.str[STRING_SET_URL],
                            va_arg(param, char *));
    data->state.url = data->set.str[STRING_SET_URL];
    break;
  case CURLOPT_PORT:
    /*
     * The port number to use when getting the URL
     */
    arg = va_arg(param, long);
    if((arg < 0) || (arg > 65535))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.use_port = arg;
    break;
  case CURLOPT_TIMEOUT:
    /*
     * The maximum time you allow curl to use for a single transfer
     * operation.
     */
    arg = va_arg(param, long);
    if((arg >= 0) && (arg <= (INT_MAX/1000)))
      data->set.timeout = arg * 1000;
    else
      return CURLE_BAD_FUNCTION_ARGUMENT;
    break;

  case CURLOPT_TIMEOUT_MS:
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.timeout = arg;
    break;

  case CURLOPT_CONNECTTIMEOUT:
    /*
     * The maximum time you allow curl to use to connect.
     */
    arg = va_arg(param, long);
    if((arg >= 0) && (arg <= (INT_MAX/1000)))
      data->set.connecttimeout = arg * 1000;
    else
      return CURLE_BAD_FUNCTION_ARGUMENT;
    break;

  case CURLOPT_CONNECTTIMEOUT_MS:
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.connecttimeout = arg;
    break;

  case CURLOPT_ACCEPTTIMEOUT_MS:
    /*
     * The maximum time you allow curl to wait for server connect
     */
    arg = va_arg(param, long);
    if(arg < 0)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.accepttimeout = arg;
    break;

  case CURLOPT_USERPWD:
    /*
     * user:password to use in the operation
     */
    result = setstropt_userpwd(va_arg(param, char *),
                               &data->set.str[STRING_USERNAME],
                               &data->set.str[STRING_PASSWORD]);
    break;

  case CURLOPT_USERNAME:
    /*
     * authentication user name to use in the operation
     */
    result = Curl_setstropt(&data->set.str[STRING_USERNAME],
                            va_arg(param, char *));
    break;
  case CURLOPT_PASSWORD:
    /*
     * authentication password to use in the operation
     */
    result = Curl_setstropt(&data->set.str[STRING_PASSWORD],
                            va_arg(param, char *));
    break;

  case CURLOPT_LOGIN_OPTIONS:
    /*
     * authentication options to use in the operation
     */
    result = Curl_setstropt(&data->set.str[STRING_OPTIONS],
                            va_arg(param, char *));
    break;

  case CURLOPT_XOAUTH2_BEARER:
    /*
     * OAuth 2.0 bearer token to use in the operation
     */
    result = Curl_setstropt(&data->set.str[STRING_BEARER],
                            va_arg(param, char *));
    break;

  case CURLOPT_POSTQUOTE:
    /*
     * List of RAW FTP commands to use after a transfer
     */
    data->set.postquote = va_arg(param, struct curl_slist *);
    break;
  case CURLOPT_PREQUOTE:
    /*
     * List of RAW FTP commands to use prior to RETR (Wesley Laxton)
     */
    data->set.prequote = va_arg(param, struct curl_slist *);
    break;
  case CURLOPT_QUOTE:
    /*
     * List of RAW FTP commands to use before a transfer
     */
    data->set.quote = va_arg(param, struct curl_slist *);
    break;
  case CURLOPT_RESOLVE:
    /*
     * List of HOST:PORT:[addresses] strings to populate the DNS cache with
     * Entries added this way will remain in the cache until explicitly
     * removed or the handle is cleaned up.
     *
     * Prefix the HOST with plus sign (+) to have the entry expire just like
     * automatically added entries.
     *
     * Prefix the HOST with dash (-) to _remove_ the entry from the cache.
     *
     * This API can remove any entry from the DNS cache, but only entries
     * that aren't actually in use right now will be pruned immediately.
     */
    data->set.resolve = va_arg(param, struct curl_slist *);
    data->state.resolve = data->set.resolve;
    break;
  case CURLOPT_PROGRESSFUNCTION:
    /*
     * Progress callback function
     */
    data->set.fprogress = va_arg(param, curl_progress_callback);
    if(data->set.fprogress)
      data->progress.callback = TRUE; /* no longer internal */
    else
      data->progress.callback = FALSE; /* NULL enforces internal */
    break;

  case CURLOPT_XFERINFOFUNCTION:
    /*
     * Transfer info callback function
     */
    data->set.fxferinfo = va_arg(param, curl_xferinfo_callback);
    if(data->set.fxferinfo)
      data->progress.callback = TRUE; /* no longer internal */
    else
      data->progress.callback = FALSE; /* NULL enforces internal */

    break;

  case CURLOPT_PROGRESSDATA:
    /*
     * Custom client data to pass to the progress callback
     */
    data->set.progress_client = va_arg(param, void *);
    break;

#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXYUSERPWD:
    /*
     * user:password needed to use the proxy
     */
    result = setstropt_userpwd(va_arg(param, char *),
                               &data->set.str[STRING_PROXYUSERNAME],
                               &data->set.str[STRING_PROXYPASSWORD]);
    break;
  case CURLOPT_PROXYUSERNAME:
    /*
     * authentication user name to use in the operation
     */
    result = Curl_setstropt(&data->set.str[STRING_PROXYUSERNAME],
                            va_arg(param, char *));
    break;
  case CURLOPT_PROXYPASSWORD:
    /*
     * authentication password to use in the operation
     */
    result = Curl_setstropt(&data->set.str[STRING_PROXYPASSWORD],
                            va_arg(param, char *));
    break;
  case CURLOPT_NOPROXY:
    /*
     * proxy exception list
     */
    result = Curl_setstropt(&data->set.str[STRING_NOPROXY],
                            va_arg(param, char *));
    break;
#endif

  case CURLOPT_RANGE:
    /*
     * What range of the file you want to transfer
     */
    result = Curl_setstropt(&data->set.str[STRING_SET_RANGE],
                            va_arg(param, char *));
    break;
  case CURLOPT_RESUME_FROM:
    /*
     * Resume transfer at the given file position
     */
    arg = va_arg(param, long);
    if(arg < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.set_resume_from = arg;
    break;
  case CURLOPT_RESUME_FROM_LARGE:
    /*
     * Resume transfer at the given file position
     */
    bigsize = va_arg(param, curl_off_t);
    if(bigsize < -1)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.set_resume_from = bigsize;
    break;
  case CURLOPT_DEBUGFUNCTION:
    /*
     * stderr write callback.
     */
    data->set.fdebug = va_arg(param, curl_debug_callback);
    /*
     * if the callback provided is NULL, it'll use the default callback
     */
    break;
  case CURLOPT_DEBUGDATA:
    /*
     * Set to a void * that should receive all error writes. This
     * defaults to CURLOPT_STDERR for normal operations.
     */
    data->set.debugdata = va_arg(param, void *);
    break;
  case CURLOPT_STDERR:
    /*
     * Set to a FILE * that should receive all error writes. This
     * defaults to stderr for normal operations.
     */
    data->set.err = va_arg(param, FILE *);
    if(!data->set.err)
      data->set.err = stderr;
    break;
  case CURLOPT_HEADERFUNCTION:
    /*
     * Set header write callback
     */
    data->set.fwrite_header = va_arg(param, curl_write_callback);
    break;
  case CURLOPT_WRITEFUNCTION:
    /*
     * Set data write callback
     */
    data->set.fwrite_func = va_arg(param, curl_write_callback);
    if(!data->set.fwrite_func) {
      data->set.is_fwrite_set = 0;
      /* When set to NULL, reset to our internal default function */
      data->set.fwrite_func = (curl_write_callback)fwrite;
    }
    else
      data->set.is_fwrite_set = 1;
    break;
  case CURLOPT_READFUNCTION:
    /*
     * Read data callback
     */
    data->set.fread_func_set = va_arg(param, curl_read_callback);
    if(!data->set.fread_func_set) {
      data->set.is_fread_set = 0;
      /* When set to NULL, reset to our internal default function */
      data->set.fread_func_set = (curl_read_callback)fread;
    }
    else
      data->set.is_fread_set = 1;
    break;
  case CURLOPT_SEEKFUNCTION:
    /*
     * Seek callback. Might be NULL.
     */
    data->set.seek_func = va_arg(param, curl_seek_callback);
    break;
  case CURLOPT_SEEKDATA:
    /*
     * Seek control callback. Might be NULL.
     */
    data->set.seek_client = va_arg(param, void *);
    break;
  case CURLOPT_CONV_FROM_NETWORK_FUNCTION:
    /*
     * "Convert from network encoding" callback
     */
    data->set.convfromnetwork = va_arg(param, curl_conv_callback);
    break;
  case CURLOPT_CONV_TO_NETWORK_FUNCTION:
    /*
     * "Convert to network encoding" callback
     */
    data->set.convtonetwork = va_arg(param, curl_conv_callback);
    break;
  case CURLOPT_CONV_FROM_UTF8_FUNCTION:
    /*
     * "Convert from UTF-8 encoding" callback
     */
    data->set.convfromutf8 = va_arg(param, curl_conv_callback);
    break;
  case CURLOPT_IOCTLFUNCTION:
    /*
     * I/O control callback. Might be NULL.
     */
    data->set.ioctl_func = va_arg(param, curl_ioctl_callback);
    break;
  case CURLOPT_IOCTLDATA:
    /*
     * I/O control data pointer. Might be NULL.
     */
    data->set.ioctl_client = va_arg(param, void *);
    break;
  case CURLOPT_SSLCERT:
    /*
     * String that holds file name of the SSL certificate to use
     */
    result = Curl_setstropt(&data->set.str[STRING_CERT],
                            va_arg(param, char *));
    break;
  case CURLOPT_SSLCERT_BLOB:
    /*
     * Blob that holds file content of the SSL certificate to use
     */
    result = Curl_setblobopt(&data->set.blobs[BLOB_CERT],
                             va_arg(param, struct curl_blob *));
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSLCERT:
    /*
     * String that holds file name of the SSL certificate to use for proxy
     */
    result = Curl_setstropt(&data->set.str[STRING_CERT_PROXY],
                            va_arg(param, char *));
    break;
  case CURLOPT_PROXY_SSLCERT_BLOB:
    /*
     * Blob that holds file content of the SSL certificate to use for proxy
     */
    result = Curl_setblobopt(&data->set.blobs[BLOB_CERT_PROXY],
                             va_arg(param, struct curl_blob *));
    break;
#endif
  case CURLOPT_SSLCERTTYPE:
    /*
     * String that holds file type of the SSL certificate to use
     */
    result = Curl_setstropt(&data->set.str[STRING_CERT_TYPE],
                            va_arg(param, char *));
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSLCERTTYPE:
    /*
     * String that holds file type of the SSL certificate to use for proxy
     */
    result = Curl_setstropt(&data->set.str[STRING_CERT_TYPE_PROXY],
                            va_arg(param, char *));
    break;
#endif
  case CURLOPT_SSLKEY:
    /*
     * String that holds file name of the SSL key to use
     */
    result = Curl_setstropt(&data->set.str[STRING_KEY],
                            va_arg(param, char *));
    break;
  case CURLOPT_SSLKEY_BLOB:
    /*
     * Blob that holds file content of the SSL key to use
     */
    result = Curl_setblobopt(&data->set.blobs[BLOB_KEY],
                             va_arg(param, struct curl_blob *));
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSLKEY:
    /*
     * String that holds file name of the SSL key to use for proxy
     */
    result = Curl_setstropt(&data->set.str[STRING_KEY_PROXY],
                            va_arg(param, char *));
    break;
  case CURLOPT_PROXY_SSLKEY_BLOB:
    /*
     * Blob that holds file content of the SSL key to use for proxy
     */
    result = Curl_setblobopt(&data->set.blobs[BLOB_KEY_PROXY],
                             va_arg(param, struct curl_blob *));
    break;
#endif
  case CURLOPT_SSLKEYTYPE:
    /*
     * String that holds file type of the SSL key to use
     */
    result = Curl_setstropt(&data->set.str[STRING_KEY_TYPE],
                            va_arg(param, char *));
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSLKEYTYPE:
    /*
     * String that holds file type of the SSL key to use for proxy
     */
    result = Curl_setstropt(&data->set.str[STRING_KEY_TYPE_PROXY],
                            va_arg(param, char *));
    break;
#endif
  case CURLOPT_KEYPASSWD:
    /*
     * String that holds the SSL or SSH private key password.
     */
    result = Curl_setstropt(&data->set.str[STRING_KEY_PASSWD],
                            va_arg(param, char *));
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_KEYPASSWD:
    /*
     * String that holds the SSL private key password for proxy.
     */
    result = Curl_setstropt(&data->set.str[STRING_KEY_PASSWD_PROXY],
                            va_arg(param, char *));
    break;
#endif
  case CURLOPT_SSLENGINE:
    /*
     * String that holds the SSL crypto engine.
     */
    argptr = va_arg(param, char *);
    if(argptr && argptr[0]) {
      result = Curl_setstropt(&data->set.str[STRING_SSL_ENGINE], argptr);
      if(!result) {
        result = Curl_ssl_set_engine(data, argptr);
      }
    }
    break;

  case CURLOPT_SSLENGINE_DEFAULT:
    /*
     * flag to set engine as default.
     */
    Curl_setstropt(&data->set.str[STRING_SSL_ENGINE], NULL);
    result = Curl_ssl_set_engine_default(data);
    break;
  case CURLOPT_CRLF:
    /*
     * Kludgy option to enable CRLF conversions. Subject for removal.
     */
    data->set.crlf = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_HAPROXYPROTOCOL:
    /*
     * Set to send the HAProxy Proxy Protocol header
     */
    data->set.haproxyprotocol = (0 != va_arg(param, long)) ? TRUE : FALSE;
    break;
#endif
  case CURLOPT_INTERFACE:
    /*
     * Set what interface or address/hostname to bind the socket to when
     * performing an operation and thus what from-IP your connection will use.
     */
    result = Curl_setstropt(&data->set.str[STRING_DEVICE],
                            va_arg(param, char *));
    break;
  case CURLOPT_LOCALPORT:
    /*
     * Set what local port to bind the socket to when performing an operation.
     */
    arg = va_arg(param, long);
    if((arg < 0) || (arg > 65535))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.localport = curlx_sltous(arg);
    break;
  case CURLOPT_LOCALPORTRANGE:
    /*
     * Set number of local ports to try, starting with CURLOPT_LOCALPORT.
     */
    arg = va_arg(param, long);
    if((arg < 0) || (arg > 65535))
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.localportrange = curlx_sltosi(arg);
    break;
  case CURLOPT_GSSAPI_DELEGATION:
    /*
     * GSS-API credential delegation bitmask
     */
    arg = va_arg(param, long);
    if(arg < CURLGSSAPI_DELEGATION_NONE)
      return CURLE_BAD_FUNCTION_ARGUMENT;
    data->set.gssapi_delegation = arg;
    break;
  case CURLOPT_SSL_VERIFYPEER:
    /*
     * Enable peer SSL verifying.
     */
    data->set.ssl.primary.verifypeer = (0 != va_arg(param, long)) ?
      TRUE : FALSE;

    /* Update the current connection ssl_config. */
    if(data->conn) {
      data->conn->ssl_config.verifypeer =
        data->set.ssl.primary.verifypeer;
    }
    break;
  case CURLOPT_DOH_SSL_VERIFYPEER:
    /*
     * Enable peer SSL verifying for DoH.
     */
    data->set.doh_verifypeer = (0 != va_arg(param, long)) ?
      TRUE : FALSE;
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSL_VERIFYPEER:
    /*
     * Enable peer SSL verifying for proxy.
     */
    data->set.proxy_ssl.primary.verifypeer =
      (0 != va_arg(param, long))?TRUE:FALSE;

    /* Update the current connection proxy_ssl_config. */
    if(data->conn) {
      data->conn->proxy_ssl_config.verifypeer =
        data->set.proxy_ssl.primary.verifypeer;
    }
    break;
#endif
  case CURLOPT_SSL_VERIFYHOST:
    /*
     * Enable verification of the host name in the peer certificate
     */
    arg = va_arg(param, long);

    /* Obviously people are not reading documentation and too many thought
       this argument took a boolean when it wasn't and misused it.
       Treat 1 and 2 the same */
    data->set.ssl.primary.verifyhost = (bool)((arg & 3) ? TRUE : FALSE);

    /* Update the current connection ssl_config. */
    if(data->conn) {
      data->conn->ssl_config.verifyhost =
        data->set.ssl.primary.verifyhost;
    }
    break;
  case CURLOPT_DOH_SSL_VERIFYHOST:
    /*
     * Enable verification of the host name in the peer certificate for DoH
     */
    arg = va_arg(param, long);

    /* Treat both 1 and 2 as TRUE */
    data->set.doh_verifyhost = (bool)((arg & 3) ? TRUE : FALSE);
    break;
#ifndef CURL_DISABLE_PROXY
  case CURLOPT_PROXY_SSL_VERIFYHOST:
    /*
     * Enable verification of the host name in the peer certificate for proxy
     */
    arg = va_arg(param, long);

    /* Treat both 1 and 2 as TRUE */
    data->set.proxy_ssl.primary.verifyhost = (bool)((arg & 3)?TRUE:FALSE);

    /* Update the current connection proxy_ssl_config. */
    if(data->conn) {
      data->conn->proxy_ssl_config.verifyhost =
        data->set.proxy_ssl.primary.verifyhost;
    }
