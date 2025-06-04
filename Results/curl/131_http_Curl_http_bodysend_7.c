CURLcode Curl_http_bodysend(struct Curl_easy *data, struct connectdata *conn,
                            struct dynbuf *r, Curl_HttpReq httpreq)
{
#ifndef USE_HYPER
  /* Hyper always handles the body separately */
  curl_off_t included_body = 0;
#endif
  CURLcode result = CURLE_OK;
  struct HTTP *http = data->req.p.http;
  const char *ptr;

  /* If 'authdone' is FALSE, we must not set the write socket index to the
     Curl_transfer() call below, as we're not ready to actually upload any
     data yet. */

  switch(httpreq) {

  case HTTPREQ_PUT: /* Let's PUT the data to the server! */

    if(conn->bits.authneg)
      http->postsize = 0;
    else
      http->postsize = data->state.infilesize;

    if((http->postsize != -1) && !data->req.upload_chunky &&
       (conn->bits.authneg || !Curl_checkheaders(data, "Content-Length"))) {
      /* only add Content-Length if not uploading chunked */
      result = Curl_dyn_addf(r, "Content-Length: %" CURL_FORMAT_CURL_OFF_T
                             "\r\n", http->postsize);
      if(result)
        return result;
    }

    if(http->postsize) {
      result = expect100(data, conn, r);
      if(result)
        return result;
    }

    /* end of headers */
    result = Curl_dyn_add(r, "\r\n");
    if(result)
      return result;

    /* set the upload size to the progress meter */
    Curl_pgrsSetUploadSize(data, http->postsize);

    /* this sends the buffer and frees all the buffer resources */
    result = Curl_buffer_send(r, data, &data->info.request_size, 0,
                              FIRSTSOCKET);
    if(result)
      failf(data, "Failed sending PUT request");
    else
      /* prepare for transfer */
      Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE,
                          http->postsize?FIRSTSOCKET:-1);
    if(result)
      return result;
    break;

  case HTTPREQ_POST_FORM:
  case HTTPREQ_POST_MIME:
    /* This is form posting using mime data. */
    if(conn->bits.authneg) {
      /* nothing to post! */
      result = Curl_dyn_add(r, "Content-Length: 0\r\n\r\n");
      if(result)
        return result;

      result = Curl_buffer_send(r, data, &data->info.request_size, 0,
                                FIRSTSOCKET);
      if(result)
        failf(data, "Failed sending POST request");
      else
        /* setup variables for the upcoming transfer */
        Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, -1);
      break;
    }

    data->state.infilesize = http->postsize;

    /* We only set Content-Length and allow a custom Content-Length if
       we don't upload data chunked, as RFC2616 forbids us to set both
       kinds of headers (Transfer-Encoding: chunked and Content-Length) */
    if(http->postsize != -1 && !data->req.upload_chunky &&
       (conn->bits.authneg || !Curl_checkheaders(data, "Content-Length"))) {
      /* we allow replacing this header if not during auth negotiation,
         although it isn't very wise to actually set your own */
      result = Curl_dyn_addf(r,
                             "Content-Length: %" CURL_FORMAT_CURL_OFF_T
                             "\r\n", http->postsize);
      if(result)
        return result;
    }

#ifndef CURL_DISABLE_MIME
    /* Output mime-generated headers. */
    {
      struct curl_slist *hdr;

      for(hdr = http->sendit->curlheaders; hdr; hdr = hdr->next) {
        result = Curl_dyn_addf(r, "%s\r\n", hdr->data);
        if(result)
          return result;
      }
    }
#endif

    /* For really small posts we don't use Expect: headers at all, and for
       the somewhat bigger ones we allow the app to disable it. Just make
       sure that the expect100header is always set to the preferred value
       here. */
    ptr = Curl_checkheaders(data, "Expect");
    if(ptr) {
      data->state.expect100header =
        Curl_compareheader(ptr, "Expect:", "100-continue");
    }
    else if(http->postsize > EXPECT_100_THRESHOLD || http->postsize < 0) {
      result = expect100(data, conn, r);
      if(result)
        return result;
    }
    else
      data->state.expect100header = FALSE;

    /* make the request end in a true CRLF */
    result = Curl_dyn_add(r, "\r\n");
    if(result)
      return result;

    /* set the upload size to the progress meter */
    Curl_pgrsSetUploadSize(data, http->postsize);

    /* Read from mime structure. */
    data->state.fread_func = (curl_read_callback) Curl_mime_read;
    data->state.in = (void *) http->sendit;
    http->sending = HTTPSEND_BODY;

    /* this sends the buffer and frees all the buffer resources */
    result = Curl_buffer_send(r, data, &data->info.request_size, 0,
                              FIRSTSOCKET);
    if(result)
      failf(data, "Failed sending POST request");
    else
      /* prepare for transfer */
      Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE,
                          http->postsize?FIRSTSOCKET:-1);
    if(result)
      return result;

    break;

  case HTTPREQ_POST:
    /* this is the simple POST, using x-www-form-urlencoded style */

    if(conn->bits.authneg)
      http->postsize = 0;
    else
      /* the size of the post body */
      http->postsize = data->state.infilesize;

    /* We only set Content-Length and allow a custom Content-Length if
       we don't upload data chunked, as RFC2616 forbids us to set both
       kinds of headers (Transfer-Encoding: chunked and Content-Length) */
    if((http->postsize != -1) && !data->req.upload_chunky &&
       (conn->bits.authneg || !Curl_checkheaders(data, "Content-Length"))) {
      /* we allow replacing this header if not during auth negotiation,
         although it isn't very wise to actually set your own */
      result = Curl_dyn_addf(r, "Content-Length: %" CURL_FORMAT_CURL_OFF_T
                             "\r\n", http->postsize);
      if(result)
        return result;
    }

    if(!Curl_checkheaders(data, "Content-Type")) {
      result = Curl_dyn_add(r, "Content-Type: application/"
                            "x-www-form-urlencoded\r\n");
      if(result)
        return result;
    }

    /* For really small posts we don't use Expect: headers at all, and for
       the somewhat bigger ones we allow the app to disable it. Just make
       sure that the expect100header is always set to the preferred value
       here. */
    ptr = Curl_checkheaders(data, "Expect");
    if(ptr) {
      data->state.expect100header =
        Curl_compareheader(ptr, "Expect:", "100-continue");
    }
    else if(http->postsize > EXPECT_100_THRESHOLD || http->postsize < 0) {
      result = expect100(data, conn, r);
      if(result)
        return result;
    }
    else
      data->state.expect100header = FALSE;

#ifndef USE_HYPER
    /* With Hyper the body is always passed on separately */
    if(data->set.postfields) {

      /* In HTTP2, we send request body in DATA frame regardless of
         its size. */
      if(conn->httpversion != 20 &&
         !data->state.expect100header &&
         (http->postsize < MAX_INITIAL_POST_SIZE)) {
        /* if we don't use expect: 100  AND
           postsize is less than MAX_INITIAL_POST_SIZE

           then append the post data to the HTTP request header. This limit
           is no magic limit but only set to prevent really huge POSTs to
           get the data duplicated with malloc() and family. */

        /* end of headers! */
        result = Curl_dyn_add(r, "\r\n");
        if(result)
          return result;

        if(!data->req.upload_chunky) {
          /* We're not sending it 'chunked', append it to the request
             already now to reduce the number if send() calls */
          result = Curl_dyn_addn(r, data->set.postfields,
                                 (size_t)http->postsize);
          included_body = http->postsize;
        }
        else {
          if(http->postsize) {
            char chunk[16];
            /* Append the POST data chunky-style */
            msnprintf(chunk, sizeof(chunk), "%x\r\n", (int)http->postsize);
            result = Curl_dyn_add(r, chunk);
            if(!result) {
              included_body = http->postsize + strlen(chunk);
              result = Curl_dyn_addn(r, data->set.postfields,
                                     (size_t)http->postsize);
              if(!result)
                result = Curl_dyn_add(r, "\r\n");
              included_body += 2;
            }
          }
          if(!result) {
            result = Curl_dyn_add(r, "\x30\x0d\x0a\x0d\x0a");
            /* 0  CR  LF  CR  LF */
            included_body += 5;
          }
        }
        if(result)
          return result;
        /* Make sure the progress information is accurate */
        Curl_pgrsSetUploadSize(data, http->postsize);
      }
      else {
        /* A huge POST coming up, do data separate from the request */
        http->postdata = data->set.postfields;

        http->sending = HTTPSEND_BODY;

        data->state.fread_func = (curl_read_callback)readmoredata;
        data->state.in = (void *)data;

        /* set the upload size to the progress meter */
        Curl_pgrsSetUploadSize(data, http->postsize);

        /* end of headers! */
        result = Curl_dyn_add(r, "\r\n");
        if(result)
          return result;
      }
    }
    else
#endif
    {
       /* end of headers! */
      result = Curl_dyn_add(r, "\r\n");
      if(result)
        return result;

      if(data->req.upload_chunky && conn->bits.authneg) {
        /* Chunky upload is selected and we're negotiating auth still, send
           end-of-data only */
        result = Curl_dyn_add(r, (char *)"\x30\x0d\x0a\x0d\x0a");
        /* 0  CR  LF  CR  LF */
        if(result)
          return result;
      }

      else if(data->state.infilesize) {
        /* set the upload size to the progress meter */
        Curl_pgrsSetUploadSize(data, http->postsize?http->postsize:-1);

        /* set the pointer to mark that we will send the post body using the
           read callback, but only if we're not in authenticate negotiation */
        if(!conn->bits.authneg)
          http->postdata = (char *)&http->postdata;
      }
    }
    /* issue the request */
    result = Curl_buffer_send(r, data, &data->info.request_size, included_body,
                              FIRSTSOCKET);

    if(result)
      failf(data, "Failed sending HTTP POST request");
    else
      Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE,
                          http->postdata?FIRSTSOCKET:-1);
    break;

  default:
    result = Curl_dyn_add(r, "\r\n");
    if(result)
      return result;

    /* issue the request */
    result = Curl_buffer_send(r, data, &data->info.request_size, 0,
                              FIRSTSOCKET);

    if(result)
      failf(data, "Failed sending HTTP request");
    else
      /* HTTP GET/HEAD download: */
      Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, -1);
  }

  return result;
}


// Source: http.c
// Lines 2372-2697
