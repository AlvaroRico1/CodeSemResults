CURLcode Curl_http_header(struct Curl_easy *data, struct connectdata *conn,
                          char *headp)
{
  CURLcode result;
  struct SingleRequest *k = &data->req;
  /* Check for Content-Length: header lines to get size */
  if(!k->http_bodyless &&
     !data->set.ignorecl && checkprefix("Content-Length:", headp)) {
    curl_off_t contentlength;
    CURLofft offt = curlx_strtoofft(headp + strlen("Content-Length:"),
                                    NULL, 10, &contentlength);

    if(offt == CURL_OFFT_OK) {
      k->size = contentlength;
      k->maxdownload = k->size;
    }
    else if(offt == CURL_OFFT_FLOW) {
      /* out of range */
      if(data->set.max_filesize) {
        failf(data, "Maximum file size exceeded");
        return CURLE_FILESIZE_EXCEEDED;
      }
      streamclose(conn, "overflow content-length");
      infof(data, "Overflow Content-Length: value!");
    }
    else {
      /* negative or just rubbish - bad HTTP */
      failf(data, "Invalid Content-Length: value");
      return CURLE_WEIRD_SERVER_REPLY;
    }
  }
  /* check for Content-Type: header lines to get the MIME-type */
  else if(checkprefix("Content-Type:", headp)) {
    char *contenttype = Curl_copy_header_value(headp);
    if(!contenttype)
      return CURLE_OUT_OF_MEMORY;
    if(!*contenttype)
      /* ignore empty data */
      free(contenttype);
    else {
      Curl_safefree(data->info.contenttype);
      data->info.contenttype = contenttype;
    }
  }
#ifndef CURL_DISABLE_PROXY
  else if((conn->httpversion == 10) &&
          conn->bits.httpproxy &&
          Curl_compareheader(headp, "Proxy-Connection:", "keep-alive")) {
    /*
     * When a HTTP/1.0 reply comes when using a proxy, the
     * 'Proxy-Connection: keep-alive' line tells us the
     * connection will be kept alive for our pleasure.
     * Default action for 1.0 is to close.
     */
    connkeep(conn, "Proxy-Connection keep-alive"); /* don't close */
    infof(data, "HTTP/1.0 proxy connection set to keep alive!");
  }
  else if((conn->httpversion == 11) &&
          conn->bits.httpproxy &&
          Curl_compareheader(headp, "Proxy-Connection:", "close")) {
    /*
     * We get a HTTP/1.1 response from a proxy and it says it'll
     * close down after this transfer.
     */
    connclose(conn, "Proxy-Connection: asked to close after done");
    infof(data, "HTTP/1.1 proxy connection set close!");
  }
#endif
  else if((conn->httpversion == 10) &&
          Curl_compareheader(headp, "Connection:", "keep-alive")) {
    /*
     * A HTTP/1.0 reply with the 'Connection: keep-alive' line
     * tells us the connection will be kept alive for our
     * pleasure.  Default action for 1.0 is to close.
     *
     * [RFC2068, section 19.7.1] */
    connkeep(conn, "Connection keep-alive");
    infof(data, "HTTP/1.0 connection set to keep alive!");
  }
  else if(Curl_compareheader(headp, "Connection:", "close")) {
    /*
     * [RFC 2616, section 8.1.2.1]
     * "Connection: close" is HTTP/1.1 language and means that
     * the connection will close when this request has been
     * served.
     */
    streamclose(conn, "Connection: close used");
  }
  else if(!k->http_bodyless && checkprefix("Transfer-Encoding:", headp)) {
    /* One or more encodings. We check for chunked and/or a compression
       algorithm. */
    /*
     * [RFC 2616, section 3.6.1] A 'chunked' transfer encoding
     * means that the server will send a series of "chunks". Each
     * chunk starts with line with info (including size of the
     * coming block) (terminated with CRLF), then a block of data
     * with the previously mentioned size. There can be any amount
     * of chunks, and a chunk-data set to zero signals the
     * end-of-chunks. */

    result = Curl_build_unencoding_stack(data,
                                         headp + strlen("Transfer-Encoding:"),
                                         TRUE);
    if(result)
      return result;
    if(!k->chunk) {
      /* if this isn't chunked, only close can signal the end of this transfer
         as Content-Length is said not to be trusted for transfer-encoding! */
      connclose(conn, "HTTP/1.1 transfer-encoding without chunks");
      k->ignore_cl = TRUE;
    }
  }
  else if(!k->http_bodyless && checkprefix("Content-Encoding:", headp) &&
          data->set.str[STRING_ENCODING]) {
    /*
     * Process Content-Encoding. Look for the values: identity,
     * gzip, deflate, compress, x-gzip and x-compress. x-gzip and
     * x-compress are the same as gzip and compress. (Sec 3.5 RFC
     * 2616). zlib cannot handle compress.  However, errors are
     * handled further down when the response body is processed
     */
    result = Curl_build_unencoding_stack(data,
                                         headp + strlen("Content-Encoding:"),
                                         FALSE);
    if(result)
      return result;
  }
  else if(checkprefix("Retry-After:", headp)) {
    /* Retry-After = HTTP-date / delay-seconds */
    curl_off_t retry_after = 0; /* zero for unknown or "now" */
    time_t date = Curl_getdate_capped(headp + strlen("Retry-After:"));
    if(-1 == date) {
      /* not a date, try it as a decimal number */
      (void)curlx_strtoofft(headp + strlen("Retry-After:"),
                            NULL, 10, &retry_after);
    }
    else
      /* convert date to number of seconds into the future */
      retry_after = date - time(NULL);
    data->info.retry_after = retry_after; /* store it */
  }
  else if(!k->http_bodyless && checkprefix("Content-Range:", headp)) {
    /* Content-Range: bytes [num]-
       Content-Range: bytes: [num]-
       Content-Range: [num]-
       Content-Range: [asterisk]/[total]

       The second format was added since Sun's webserver
       JavaWebServer/1.1.1 obviously sends the header this way!
       The third added since some servers use that!
       The forth means the requested range was unsatisfied.
    */

    char *ptr = headp + strlen("Content-Range:");

    /* Move forward until first digit or asterisk */
    while(*ptr && !ISDIGIT(*ptr) && *ptr != '*')
      ptr++;

    /* if it truly stopped on a digit */
    if(ISDIGIT(*ptr)) {
      if(!curlx_strtoofft(ptr, NULL, 10, &k->offset)) {
        if(data->state.resume_from == k->offset)
          /* we asked for a resume and we got it */
          k->content_range = TRUE;
      }
    }
    else
      data->state.resume_from = 0; /* get everything */
  }


// Source: http.c
// Lines 3407-3576
