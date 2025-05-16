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


// Source: setopt.c
// Lines 160-438
