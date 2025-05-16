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


// Source: http.c
// Lines 2372-2477
