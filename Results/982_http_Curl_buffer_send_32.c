// Source: curl/lib/http.c
// Lines 1232-1235
CURLcode Curl_buffer_send(struct dynbuf *in,
                          struct Curl_easy *data,
                          /* add the number of sent bytes to this
                             counter */
                          curl_off_t *bytes_written,
                          /* how much of the buffer contains body data */
                          curl_off_t included_body_bytes,
                          int socketindex)
{
  ssize_t amount;
  CURLcode result;
  char *ptr;
  size_t size;
  struct connectdata *conn = data->conn;
  struct HTTP *http = data->req.p.http;
  size_t sendsize;
  curl_socket_t sockfd;
  size_t headersize;

  DEBUGASSERT(socketindex <= SECONDARYSOCKET);

  sockfd = conn->sock[socketindex];

  /* The looping below is required since we use non-blocking sockets, but due
     to the circumstances we will just loop and try again and again etc */

  ptr = Curl_dyn_ptr(in);
  size = Curl_dyn_len(in);

  headersize = size - (size_t)included_body_bytes; /* the initial part that
                                                      isn't body is header */

  DEBUGASSERT(size > (size_t)included_body_bytes);

  result = Curl_convert_to_network(data, ptr, headersize);
  /* Curl_convert_to_network calls failf if unsuccessful */
  if(result) {
    /* conversion failed, free memory and return to the caller */
    Curl_dyn_free(in);
    return result;
  }

  if((conn->handler->flags & PROTOPT_SSL
#ifndef CURL_DISABLE_PROXY
      || conn->http_proxy.proxytype == CURLPROXY_HTTPS
#endif
       )
     && conn->httpversion != 20) {
    /* Make sure this doesn't send more body bytes than what the max send
       speed says. The request bytes do not count to the max speed.
    */
    if(data->set.max_send_speed &&
       (included_body_bytes > data->set.max_send_speed)) {
      curl_off_t overflow = included_body_bytes - data->set.max_send_speed;
      DEBUGASSERT((size_t)overflow < size);
      sendsize = size - (size_t)overflow;
    }
    else
      sendsize = size;

    /* OpenSSL is very picky and we must send the SAME buffer pointer to the
       library when we attempt to re-send this buffer. Sending the same data
       is not enough, we must use the exact same address. For this reason, we
       must copy the data to the uploadbuffer first, since that is the buffer
       we will be using if this send is retried later.
    */
    result = Curl_get_upload_buffer(data);
    if(result) {
      /* malloc failed, free memory and return to the caller */
      Curl_dyn_free(in);
      return result;
    }
    /* We never send more than upload_buffer_size bytes in one single chunk
       when we speak HTTPS, as if only a fraction of it is sent now, this data
       needs to fit into the normal read-callback buffer later on and that
       buffer is using this size.
    */
    if(sendsize > (size_t)data->set.upload_buffer_size)
      sendsize = (size_t)data->set.upload_buffer_size;

    memcpy(data->state.ulbuf, ptr, sendsize);
    ptr = data->state.ulbuf;
  }
  else {
#ifdef CURLDEBUG
    /* Allow debug builds to override this logic to force short initial
       sends
     */
    char *p = getenv("CURL_SMALLREQSEND");
    if(p) {
      size_t altsize = (size_t)strtoul(p, NULL, 10);
      if(altsize)
        sendsize = CURLMIN(size, altsize);
      else
        sendsize = size;
    }
    else
#endif
    {
      /* Make sure this doesn't send more body bytes than what the max send
         speed says. The request bytes do not count to the max speed.
      */
      if(data->set.max_send_speed &&
         (included_body_bytes > data->set.max_send_speed)) {
        curl_off_t overflow = included_body_bytes - data->set.max_send_speed;
        DEBUGASSERT((size_t)overflow < size);
        sendsize = size - (size_t)overflow;
      }
      else
        sendsize = size;
    }
  }

  result = Curl_write(data, sockfd, ptr, sendsize, &amount);

  if(!result) {
    /*
     * Note that we may not send the entire chunk at once, and we have a set
     * number of data bytes at the end of the big buffer (out of which we may
     * only send away a part).
     */
    /* how much of the header that was sent */
    size_t headlen = (size_t)amount>headersize ? headersize : (size_t)amount;
    size_t bodylen = amount - headlen;

    /* this data _may_ contain binary stuff */
    Curl_debug(data, CURLINFO_HEADER_OUT, ptr, headlen);
    if(bodylen)
      /* there was body data sent beyond the initial header part, pass that on
         to the debug callback too */
      Curl_debug(data, CURLINFO_DATA_OUT, ptr + headlen, bodylen);

    /* 'amount' can never be a very large value here so typecasting it so a
       signed 31 bit value should not cause problems even if ssize_t is
       64bit */
    *bytes_written += (long)amount;

    if(http) {
      /* if we sent a piece of the body here, up the byte counter for it
         accordingly */
      data->req.writebytecount += bodylen;
      Curl_pgrsSetUploadCounter(data, data->req.writebytecount);

      if((size_t)amount != size) {
        /* The whole request could not be sent in one system call. We must
           queue it up and send it later when we get the chance. We must not
           loop here and wait until it might work again. */

        size -= amount;

        ptr = Curl_dyn_ptr(in) + amount;

        /* backup the currently set pointers */
        http->backup.fread_func = data->state.fread_func;
        http->backup.fread_in = data->state.in;
        http->backup.postdata = http->postdata;
        http->backup.postsize = http->postsize;

        /* set the new pointers for the request-sending */
        data->state.fread_func = (curl_read_callback)readmoredata;
        data->state.in = (void *)data;
        http->postdata = ptr;
        http->postsize = (curl_off_t)size;

        /* this much data is remaining header: */
        data->req.pendingheader = headersize - headlen;

        http->send_buffer = *in; /* copy the whole struct */
        http->sending = HTTPSEND_REQUEST;

        return CURLE_OK;
      }
      http->sending = HTTPSEND_BODY;
      /* the full buffer was sent, clean up and return */
    }
    else {
      if((size_t)amount != size)
        /* We have no continue-send mechanism now, fail. This can only happen
           when this function is used from the CONNECT sending function. We
           currently (stupidly) assume that the whole request is always sent
           away in the first single chunk.

           This needs FIXing.
        */
        return CURLE_SEND_ERROR;
    }
  }
  Curl_dyn_free(in);

  /* no remaining header data */
  data->req.pendingheader = 0;
  return result;
}
