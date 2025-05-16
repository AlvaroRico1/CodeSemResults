CURLcode Curl_fillreadbuffer(struct Curl_easy *data, size_t bytes,
                             size_t *nreadp)
{
  size_t buffersize = bytes;
  size_t nread;

  curl_read_callback readfunc = NULL;
  void *extra_data = NULL;

#ifdef CURL_DOES_CONVERSIONS
  bool sending_http_headers = FALSE;
  struct connectdata *conn = data->conn;

  if(conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_RTSP)) {
    const struct HTTP *http = data->req.p.http;

    if(http->sending == HTTPSEND_REQUEST)
      /* We're sending the HTTP request headers, not the data.
         Remember that so we don't re-translate them into garbage. */
      sending_http_headers = TRUE;
  }
#endif

#ifndef CURL_DISABLE_HTTP
  if(data->state.trailers_state == TRAILERS_INITIALIZED) {
    struct curl_slist *trailers = NULL;
    CURLcode result;
    int trailers_ret_code;

    /* at this point we already verified that the callback exists
       so we compile and store the trailers buffer, then proceed */
    infof(data,
          "Moving trailers state machine from initialized to sending.");
    data->state.trailers_state = TRAILERS_SENDING;
    Curl_dyn_init(&data->state.trailers_buf, DYN_TRAILERS);

    data->state.trailers_bytes_sent = 0;
    Curl_set_in_callback(data, true);
    trailers_ret_code = data->set.trailer_callback(&trailers,
                                                   data->set.trailer_data);
    Curl_set_in_callback(data, false);
    if(trailers_ret_code == CURL_TRAILERFUNC_OK) {
      result = Curl_http_compile_trailers(trailers, &data->state.trailers_buf,
                                          data);
    }
    else {
      failf(data, "operation aborted by trailing headers callback");
      *nreadp = 0;
      result = CURLE_ABORTED_BY_CALLBACK;
    }
    if(result) {
      Curl_dyn_free(&data->state.trailers_buf);
      curl_slist_free_all(trailers);
      return result;
    }
    infof(data, "Successfully compiled trailers.");
    curl_slist_free_all(trailers);
  }
#endif

  /* if we are transmitting trailing data, we don't need to write
     a chunk size so we skip this */
  if(data->req.upload_chunky &&
     data->state.trailers_state == TRAILERS_NONE) {
    /* if chunked Transfer-Encoding */
    buffersize -= (8 + 2 + 2);   /* 32bit hex + CRLF + CRLF */
    data->req.upload_fromhere += (8 + 2); /* 32bit hex + CRLF */
  }

#ifndef CURL_DISABLE_HTTP
  if(data->state.trailers_state == TRAILERS_SENDING) {
    /* if we're here then that means that we already sent the last empty chunk
       but we didn't send a final CR LF, so we sent 0 CR LF. We then start
       pulling trailing data until we have no more at which point we
       simply return to the previous point in the state machine as if
       nothing happened.
       */
    readfunc = trailers_read;
    extra_data = (void *)data;
  }
  else
#endif
  {
    readfunc = data->state.fread_func;
    extra_data = data->state.in;
  }

  Curl_set_in_callback(data, true);
  nread = readfunc(data->req.upload_fromhere, 1,
                   buffersize, extra_data);
  Curl_set_in_callback(data, false);

  if(nread == CURL_READFUNC_ABORT) {
    failf(data, "operation aborted by callback");
    *nreadp = 0;
    return CURLE_ABORTED_BY_CALLBACK;
  }
  if(nread == CURL_READFUNC_PAUSE) {
    struct SingleRequest *k = &data->req;

    if(data->conn->handler->flags & PROTOPT_NONETWORK) {
      /* protocols that work without network cannot be paused. This is
         actually only FILE:// just now, and it can't pause since the transfer
         isn't done using the "normal" procedure. */
      failf(data, "Read callback asked for PAUSE when not supported!");
      return CURLE_READ_ERROR;
    }

    /* CURL_READFUNC_PAUSE pauses read callbacks that feed socket writes */
    k->keepon |= KEEP_SEND_PAUSE; /* mark socket send as paused */
    if(data->req.upload_chunky) {
        /* Back out the preallocation done above */
      data->req.upload_fromhere -= (8 + 2);
    }
    *nreadp = 0;

    return CURLE_OK; /* nothing was read */
  }
  else if(nread > buffersize) {
    /* the read function returned a too large value */
    *nreadp = 0;
    failf(data, "read function returned funny value");
    return CURLE_READ_ERROR;
  }

  if(!data->req.forbidchunk && data->req.upload_chunky) {
    /* if chunked Transfer-Encoding
     *    build chunk:
     *
     *        <HEX SIZE> CRLF
     *        <DATA> CRLF
     */
    /* On non-ASCII platforms the <DATA> may or may not be
       translated based on state.prefer_ascii while the protocol
       portion must always be translated to the network encoding.
       To further complicate matters, line end conversion might be
       done later on, so we need to prevent CRLFs from becoming
       CRCRLFs if that's the case.  To do this we use bare LFs
       here, knowing they'll become CRLFs later on.
     */

    bool added_crlf = FALSE;
    int hexlen = 0;
    const char *endofline_native;
    const char *endofline_network;

    if(
#ifdef CURL_DO_LINEEND_CONV
       (data->state.prefer_ascii) ||
#endif
       (data->set.crlf)) {
      /* \n will become \r\n later on */
      endofline_native  = "\n";
      endofline_network = "\x0a";
    }
    else {
      endofline_native  = "\r\n";
      endofline_network = "\x0d\x0a";
    }

    /* if we're not handling trailing data, proceed as usual */
    if(data->state.trailers_state != TRAILERS_SENDING) {
      char hexbuffer[11] = "";
      hexlen = msnprintf(hexbuffer, sizeof(hexbuffer),
                         "%zx%s", nread, endofline_native);

      /* move buffer pointer */
      data->req.upload_fromhere -= hexlen;
      nread += hexlen;

      /* copy the prefix to the buffer, leaving out the NUL */
      memcpy(data->req.upload_fromhere, hexbuffer, hexlen);

      /* always append ASCII CRLF to the data unless
         we have a valid trailer callback */
#ifndef CURL_DISABLE_HTTP
      if((nread-hexlen) == 0 &&
          data->set.trailer_callback != NULL &&
          data->state.trailers_state == TRAILERS_NONE) {
        data->state.trailers_state = TRAILERS_INITIALIZED;
      }
      else
#endif
      {
        memcpy(data->req.upload_fromhere + nread,
               endofline_network,
               strlen(endofline_network));
        added_crlf = TRUE;
      }
    }

#ifdef CURL_DOES_CONVERSIONS
    {
      CURLcode result;
      size_t length;
      if(data->state.prefer_ascii)
        /* translate the protocol and data */
        length = nread;
      else
        /* just translate the protocol portion */
        length = hexlen;
      if(length) {
        result = Curl_convert_to_network(data, data->req.upload_fromhere,
                                         length);
        /* Curl_convert_to_network calls failf if unsuccessful */
        if(result)
          return result;
      }
    }
#endif /* CURL_DOES_CONVERSIONS */

#ifndef CURL_DISABLE_HTTP
    if(data->state.trailers_state == TRAILERS_SENDING &&
       !trailers_left(data)) {
      Curl_dyn_free(&data->state.trailers_buf);
      data->state.trailers_state = TRAILERS_DONE;
      data->set.trailer_data = NULL;
      data->set.trailer_callback = NULL;
      /* mark the transfer as done */
      data->req.upload_done = TRUE;
      infof(data, "Signaling end of chunked upload after trailers.");
    }
    else
#endif
      if((nread - hexlen) == 0 &&
         data->state.trailers_state != TRAILERS_INITIALIZED) {
        /* mark this as done once this chunk is transferred */
        data->req.upload_done = TRUE;
        infof(data,
              "Signaling end of chunked upload via terminating chunk.");
      }

    if(added_crlf)
      nread += strlen(endofline_network); /* for the added end of line */
  }
#ifdef CURL_DOES_CONVERSIONS
  else if((data->state.prefer_ascii) && (!sending_http_headers)) {
    CURLcode result;
    result = Curl_convert_to_network(data, data->req.upload_fromhere, nread);
    /* Curl_convert_to_network calls failf if unsuccessful */
    if(result)
      return result;
  }
#endif /* CURL_DOES_CONVERSIONS */

  *nreadp = nread;

  return CURLE_OK;
}


// Source: transfer.c
// Lines 159-407
