CURLcode Curl_http_readwrite_headers(struct Curl_easy *data,
                                     struct connectdata *conn,
                                     ssize_t *nread,
                                     bool *stop_reading)
{
  CURLcode result;
  struct SingleRequest *k = &data->req;
  ssize_t onread = *nread;
  char *ostr = k->str;
  char *headp;
  char *str_start;
  char *end_ptr;

  /* header line within buffer loop */
  do {
    size_t rest_length;
    size_t full_length;
    int writetype;

    /* str_start is start of line within buf */
    str_start = k->str;

    /* data is in network encoding so use 0x0a instead of '\n' */
    end_ptr = memchr(str_start, 0x0a, *nread);

    if(!end_ptr) {
      /* Not a complete header line within buffer, append the data to
         the end of the headerbuff. */
      result = Curl_dyn_addn(&data->state.headerb, str_start, *nread);
      if(result)
        return result;

      if(!k->headerline) {
        /* check if this looks like a protocol header */
        statusline st =
          checkprotoprefix(data, conn,
                           Curl_dyn_ptr(&data->state.headerb),
                           Curl_dyn_len(&data->state.headerb));

        if(st == STATUS_BAD) {
          /* this is not the beginning of a protocol first header line */
          k->header = FALSE;
          k->badheader = HEADER_ALLBAD;
          streamclose(conn, "bad HTTP: No end-of-message indicator");
          if(!data->set.http09_allowed) {
            failf(data, "Received HTTP/0.9 when not allowed");
            return CURLE_UNSUPPORTED_PROTOCOL;
          }
          break;
        }
      }

      break; /* read more and try again */
    }

    /* decrease the size of the remaining (supposed) header line */
    rest_length = (end_ptr - k->str) + 1;
    *nread -= (ssize_t)rest_length;

    k->str = end_ptr + 1; /* move past new line */

    full_length = k->str - str_start;

    result = Curl_dyn_addn(&data->state.headerb, str_start, full_length);
    if(result)
      return result;

    /****
     * We now have a FULL header line in 'headerb'.
     *****/

    if(!k->headerline) {
      /* the first read header */
      statusline st = checkprotoprefix(data, conn,
                                       Curl_dyn_ptr(&data->state.headerb),
                                       Curl_dyn_len(&data->state.headerb));
      if(st == STATUS_BAD) {
        streamclose(conn, "bad HTTP: No end-of-message indicator");
        /* this is not the beginning of a protocol first header line */
        if(!data->set.http09_allowed) {
          failf(data, "Received HTTP/0.9 when not allowed");
          return CURLE_UNSUPPORTED_PROTOCOL;
        }
        k->header = FALSE;
        if(*nread)
          /* since there's more, this is a partial bad header */
          k->badheader = HEADER_PARTHEADER;
        else {
          /* this was all we read so it's all a bad header */
          k->badheader = HEADER_ALLBAD;
          *nread = onread;
          k->str = ostr;
          return CURLE_OK;
        }
        break;
      }
    }

    /* headers are in network encoding so use 0x0a and 0x0d instead of '\n'
       and '\r' */
    headp = Curl_dyn_ptr(&data->state.headerb);
    if((0x0a == *headp) || (0x0d == *headp)) {
      size_t headerlen;
      /* Zero-length header line means end of headers! */

#ifdef CURL_DOES_CONVERSIONS
      if(0x0d == *headp) {
        *headp = '\r'; /* replace with CR in host encoding */
        headp++;       /* pass the CR byte */
      }
      if(0x0a == *headp) {
        *headp = '\n'; /* replace with LF in host encoding */
        headp++;       /* pass the LF byte */
      }
#else
      if('\r' == *headp)
        headp++; /* pass the \r byte */
      if('\n' == *headp)
        headp++; /* pass the \n byte */
#endif /* CURL_DOES_CONVERSIONS */

      if(100 <= k->httpcode && 199 >= k->httpcode) {
        /* "A user agent MAY ignore unexpected 1xx status responses." */
        switch(k->httpcode) {
        case 100:
          /*
           * We have made a HTTP PUT or POST and this is 1.1-lingo
           * that tells us that the server is OK with this and ready
           * to receive the data.
           * However, we'll get more headers now so we must get
           * back into the header-parsing state!
           */
          k->header = TRUE;
          k->headerline = 0; /* restart the header line counter */

          /* if we did wait for this do enable write now! */
          if(k->exp100 > EXP100_SEND_DATA) {
            k->exp100 = EXP100_SEND_DATA;
            k->keepon |= KEEP_SEND;
            Curl_expire_done(data, EXPIRE_100_TIMEOUT);
          }
          break;
        case 101:
          /* Switching Protocols */
          if(k->upgr101 == UPGR101_REQUESTED) {
            /* Switching to HTTP/2 */
            infof(data, "Received 101");
            k->upgr101 = UPGR101_RECEIVED;

            /* we'll get more headers (HTTP/2 response) */
            k->header = TRUE;
            k->headerline = 0; /* restart the header line counter */

            /* switch to http2 now. The bytes after response headers
               are also processed here, otherwise they are lost. */
            result = Curl_http2_switched(data, k->str, *nread);
            if(result)
              return result;
            *nread = 0;
          }
          else {
            /* Switching to another protocol (e.g. WebSocket) */
            k->header = FALSE; /* no more header to parse! */
          }
          break;
        default:
          /* the status code 1xx indicates a provisional response, so
             we'll get another set of headers */
          k->header = TRUE;
          k->headerline = 0; /* restart the header line counter */
          break;
        }
      }
      else {
        k->header = FALSE; /* no more header to parse! */

        if((k->size == -1) && !k->chunk && !conn->bits.close &&
           (conn->httpversion == 11) &&
           !(conn->handler->protocol & CURLPROTO_RTSP) &&
           data->state.httpreq != HTTPREQ_HEAD) {
          /* On HTTP 1.1, when connection is not to get closed, but no
             Content-Length nor Transfer-Encoding chunked have been
             received, according to RFC2616 section 4.4 point 5, we
             assume that the server will close the connection to
             signal the end of the document. */
          infof(data, "no chunk, no close, no size. Assume close to "
                "signal end");
          streamclose(conn, "HTTP: No end-of-message indicator");
        }
      }

      /* At this point we have some idea about the fate of the connection.
         If we are closing the connection it may result auth failure. */
#if defined(USE_NTLM)
      if(conn->bits.close &&
         (((data->req.httpcode == 401) &&
           (conn->http_ntlm_state == NTLMSTATE_TYPE2)) ||
          ((data->req.httpcode == 407) &&
           (conn->proxy_ntlm_state == NTLMSTATE_TYPE2)))) {
        infof(data, "Connection closure while negotiating auth (HTTP 1.0?)");
        data->state.authproblem = TRUE;
      }
#endif
#if defined(USE_SPNEGO)
      if(conn->bits.close &&
        (((data->req.httpcode == 401) &&
          (conn->http_negotiate_state == GSS_AUTHRECV)) ||
         ((data->req.httpcode == 407) &&
          (conn->proxy_negotiate_state == GSS_AUTHRECV)))) {
        infof(data, "Connection closure while negotiating auth (HTTP 1.0?)");
        data->state.authproblem = TRUE;
      }
      if((conn->http_negotiate_state == GSS_AUTHDONE) &&
         (data->req.httpcode != 401)) {
        conn->http_negotiate_state = GSS_AUTHSUCC;
      }
      if((conn->proxy_negotiate_state == GSS_AUTHDONE) &&
         (data->req.httpcode != 407)) {
        conn->proxy_negotiate_state = GSS_AUTHSUCC;
      }
#endif

      /* now, only output this if the header AND body are requested:
       */
      writetype = CLIENTWRITE_HEADER;
      if(data->set.include_header)
        writetype |= CLIENTWRITE_BODY;

      headerlen = Curl_dyn_len(&data->state.headerb);
      result = Curl_client_write(data, writetype,
                                 Curl_dyn_ptr(&data->state.headerb),
                                 headerlen);
      if(result)
        return result;

      data->info.header_size += (long)headerlen;
      data->req.headerbytecount += (long)headerlen;

      /*
       * When all the headers have been parsed, see if we should give
       * up and return an error.
       */
      if(http_should_fail(data)) {
        failf(data, "The requested URL returned error: %d",
              k->httpcode);
        return CURLE_HTTP_RETURNED_ERROR;
      }

      data->req.deductheadercount =
        (100 <= k->httpcode && 199 >= k->httpcode)?data->req.headerbytecount:0;

      /* Curl_http_auth_act() checks what authentication methods
       * that are available and decides which one (if any) to
       * use. It will set 'newurl' if an auth method was picked. */
      result = Curl_http_auth_act(data);

      if(result)
        return result;

      if(k->httpcode >= 300) {
        if((!conn->bits.authneg) && !conn->bits.close &&
           !conn->bits.rewindaftersend) {
          /*
           * General treatment of errors when about to send data. Including :
           * "417 Expectation Failed", while waiting for 100-continue.
           *
           * The check for close above is done simply because of something
           * else has already deemed the connection to get closed then
           * something else should've considered the big picture and we
           * avoid this check.
           *
           * rewindaftersend indicates that something has told libcurl to
           * continue sending even if it gets discarded
           */

          switch(data->state.httpreq) {
          case HTTPREQ_PUT:
          case HTTPREQ_POST:
          case HTTPREQ_POST_FORM:
          case HTTPREQ_POST_MIME:
            /* We got an error response. If this happened before the whole
             * request body has been sent we stop sending and mark the
             * connection for closure after we've read the entire response.
             */
            Curl_expire_done(data, EXPIRE_100_TIMEOUT);
            if(!k->upload_done) {
              if((k->httpcode == 417) && data->state.expect100header) {
                /* 417 Expectation Failed - try again without the Expect
                   header */
                infof(data, "Got 417 while waiting for a 100");
                data->state.disableexpect = TRUE;
                DEBUGASSERT(!data->req.newurl);
                data->req.newurl = strdup(data->state.url);
                Curl_done_sending(data, k);
              }
              else if(data->set.http_keep_sending_on_error) {
                infof(data, "HTTP error before end of send, keep sending");
                if(k->exp100 > EXP100_SEND_DATA) {
                  k->exp100 = EXP100_SEND_DATA;
                  k->keepon |= KEEP_SEND;
                }
              }
              else {
                infof(data, "HTTP error before end of send, stop sending");
                streamclose(conn, "Stop sending data before everything sent");
                result = Curl_done_sending(data, k);
                if(result)
                  return result;
                k->upload_done = TRUE;
                if(data->state.expect100header)
                  k->exp100 = EXP100_FAILED;
              }
            }
            break;

          default: /* default label present to avoid compiler warnings */
            break;
          }
        }

        if(conn->bits.rewindaftersend) {
          /* We rewind after a complete send, so thus we continue
             sending now */
          infof(data, "Keep sending data to get tossed away!");
          k->keepon |= KEEP_SEND;
        }
      }

      if(!k->header) {
        /*
         * really end-of-headers.
         *
         * If we requested a "no body", this is a good time to get
         * out and return home.
         */
        if(data->set.opt_no_body)
          *stop_reading = TRUE;
#ifndef CURL_DISABLE_RTSP
        else if((conn->handler->protocol & CURLPROTO_RTSP) &&
                (data->set.rtspreq == RTSPREQ_DESCRIBE) &&
                (k->size <= -1))
          /* Respect section 4.4 of rfc2326: If the Content-Length header is
             absent, a length 0 must be assumed.  It will prevent libcurl from
             hanging on DESCRIBE request that got refused for whatever
             reason */
          *stop_reading = TRUE;
#endif
        else {
          /* If we know the expected size of this document, we set the
             maximum download size to the size of the expected
             document or else, we won't know when to stop reading!

             Note that we set the download maximum even if we read a
             "Connection: close" header, to make sure that
             "Content-Length: 0" still prevents us from attempting to
             read the (missing) response-body.
          */
          /* According to RFC2616 section 4.4, we MUST ignore
             Content-Length: headers if we are now receiving data
             using chunked Transfer-Encoding.
          */
          if(k->chunk)
            k->maxdownload = k->size = -1;
        }
        if(-1 != k->size) {
          /* We do this operation even if no_body is true, since this
             data might be retrieved later with curl_easy_getinfo()
             and its CURLINFO_CONTENT_LENGTH_DOWNLOAD option. */

          Curl_pgrsSetDownloadSize(data, k->size);
          k->maxdownload = k->size;
        }

        /* If max download size is *zero* (nothing) we already have
           nothing and can safely return ok now!  But for HTTP/2, we'd
           like to call http2_handle_stream_close to properly close a
           stream.  In order to do this, we keep reading until we
           close the stream. */
        if(0 == k->maxdownload
#if defined(USE_NGHTTP2)
           && !((conn->handler->protocol & PROTO_FAMILY_HTTP) &&
                conn->httpversion == 20)
#endif
           )
          *stop_reading = TRUE;

        if(*stop_reading) {
          /* we make sure that this socket isn't read more now */
          k->keepon &= ~KEEP_RECV;
        }

        Curl_debug(data, CURLINFO_HEADER_IN, str_start, headerlen);
        break; /* exit header line loop */
      }

      /* We continue reading headers, reset the line-based header */
      Curl_dyn_reset(&data->state.headerb);
      continue;
    }

    /*
     * Checks for special headers coming up.
     */

    if(!k->headerline++) {
      /* This is the first header, it MUST be the error code line
         or else we consider this to be the body right away! */
      int httpversion_major;
      int rtspversion_major;
      int nc = 0;
#ifdef CURL_DOES_CONVERSIONS
#define HEADER1 scratch
#define SCRATCHSIZE 21
      CURLcode res;
      char scratch[SCRATCHSIZE + 1]; /* "HTTP/major.minor 123" */
      /* We can't really convert this yet because we don't know if it's the
         1st header line or the body.  So we do a partial conversion into a
         scratch area, leaving the data at 'headp' as-is.
      */
      strncpy(&scratch[0], headp, SCRATCHSIZE);
      scratch[SCRATCHSIZE] = 0; /* null terminate */
      res = Curl_convert_from_network(data,
                                      &scratch[0],
                                      SCRATCHSIZE);
      if(res)
        /* Curl_convert_from_network calls failf if unsuccessful */
        return res;
#else
#define HEADER1 headp /* no conversion needed, just use headp */
#endif /* CURL_DOES_CONVERSIONS */

      if(conn->handler->protocol & PROTO_FAMILY_HTTP) {
        /*
         * https://tools.ietf.org/html/rfc7230#section-3.1.2
         *
         * The response code is always a three-digit number in HTTP as the spec
         * says. We allow any three-digit number here, but we cannot make
         * guarantees on future behaviors since it isn't within the protocol.
         */
        char separator;
        char twoorthree[2];
        int httpversion = 0;
        int digit4 = -1; /* should remain untouched to be good */
        nc = sscanf(HEADER1,
                    " HTTP/%1d.%1d%c%3d%1d",
                    &httpversion_major,
                    &httpversion,
                    &separator,
                    &k->httpcode,
                    &digit4);

        if(nc == 1 && httpversion_major >= 2 &&
           2 == sscanf(HEADER1, " HTTP/%1[23] %d", twoorthree, &k->httpcode)) {
          conn->httpversion = 0;
          nc = 4;
          separator = ' ';
        }

        /* There can only be a 4th response code digit stored in 'digit4' if
           all the other fields were parsed and stored first, so nc is 5 when
           digit4 is not -1 */
        else if(digit4 != -1) {
          failf(data, "Unsupported response code in HTTP response");
          return CURLE_UNSUPPORTED_PROTOCOL;
        }

        if((nc == 4) && (' ' == separator)) {
          httpversion += 10 * httpversion_major;
          switch(httpversion) {
          case 10:
          case 11:
#if defined(USE_NGHTTP2) || defined(USE_HYPER)
          case 20:
#endif
#if defined(ENABLE_QUIC)
          case 30:
#endif
            conn->httpversion = (unsigned char)httpversion;
            break;
          default:
            failf(data, "Unsupported HTTP version (%u.%d) in response",
                  httpversion/10, httpversion%10);
            return CURLE_UNSUPPORTED_PROTOCOL;
          }

          if(k->upgr101 == UPGR101_RECEIVED) {
            /* supposedly upgraded to http2 now */
            if(conn->httpversion != 20)
              infof(data, "Lying server, not serving HTTP/2");
          }
          if(conn->httpversion < 20) {
            conn->bundle->multiuse = BUNDLE_NO_MULTIUSE;
            infof(data, "Mark bundle as not supporting multiuse");
          }
        }
        else if(!nc) {
          /* this is the real world, not a Nirvana
             NCSA 1.5.x returns this crap when asked for HTTP/1.1
          */
          nc = sscanf(HEADER1, " HTTP %3d", &k->httpcode);
          conn->httpversion = 10;

          /* If user has set option HTTP200ALIASES,
             compare header line against list of aliases
          */
          if(!nc) {
            statusline check =
              checkhttpprefix(data,
                              Curl_dyn_ptr(&data->state.headerb),
                              Curl_dyn_len(&data->state.headerb));
            if(check == STATUS_DONE) {
              nc = 1;
              k->httpcode = 200;
              conn->httpversion = 10;
            }
          }
        }
        else {
          failf(data, "Unsupported HTTP version in response");
          return CURLE_UNSUPPORTED_PROTOCOL;
        }
      }
      else if(conn->handler->protocol & CURLPROTO_RTSP) {
        char separator;
        int rtspversion;
        nc = sscanf(HEADER1,
                    " RTSP/%1d.%1d%c%3d",
                    &rtspversion_major,
                    &rtspversion,
                    &separator,
                    &k->httpcode);
        if((nc == 4) && (' ' == separator)) {
          conn->httpversion = 11; /* For us, RTSP acts like HTTP 1.1 */
        }
        else {
          nc = 0;
        }
      }

      if(nc) {
        result = Curl_http_statusline(data, conn);
        if(result)
          return result;
      }
      else {
        k->header = FALSE;   /* this is not a header line */
        break;
      }
    }

    result = Curl_convert_from_network(data, headp, strlen(headp));
    /* Curl_convert_from_network calls failf if unsuccessful */
    if(result)
      return result;

    result = Curl_http_header(data, conn, headp);
    if(result)
      return result;

    /*
     * End of header-checks. Write them to the client.
     */

    writetype = CLIENTWRITE_HEADER;
    if(data->set.include_header)
      writetype |= CLIENTWRITE_BODY;

    Curl_debug(data, CURLINFO_HEADER_IN, headp,
               Curl_dyn_len(&data->state.headerb));

    result = Curl_client_write(data, writetype, headp,
                               Curl_dyn_len(&data->state.headerb));
    if(result)
      return result;

    data->info.header_size += Curl_dyn_len(&data->state.headerb);
    data->req.headerbytecount += Curl_dyn_len(&data->state.headerb);

    Curl_dyn_reset(&data->state.headerb);
  }
  while(*k->str); /* header line within buffer */

  /* We might have reached the end of the header part here, but
     there might be a non-header part left in the end of the read
     buffer. */

  return CURLE_OK;
}


// Source: http.c
// Lines 3793-4380
