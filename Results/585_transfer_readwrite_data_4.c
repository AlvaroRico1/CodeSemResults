static CURLcode readwrite_data(struct Curl_easy *data,
                               struct connectdata *conn,
                               struct SingleRequest *k,
                               int *didwhat, bool *done,
                               bool *comeback)
{
  CURLcode result = CURLE_OK;
  ssize_t nread; /* number of bytes read */
  size_t excess = 0; /* excess bytes read */
  bool readmore = FALSE; /* used by RTP to signal for more data */
  int maxloops = 100;
  char *buf = data->state.buffer;
  DEBUGASSERT(buf);

  *done = FALSE;
  *comeback = FALSE;

  /* This is where we loop until we have read everything there is to
     read or we get a CURLE_AGAIN */
  do {
    bool is_empty_data = FALSE;
    size_t buffersize = data->set.buffer_size;
    size_t bytestoread = buffersize;
#ifdef USE_NGHTTP2
    bool is_http2 = ((conn->handler->protocol & PROTO_FAMILY_HTTP) &&
                     (conn->httpversion == 20));
#endif

    if(
#ifdef USE_NGHTTP2
       /* For HTTP/2, read data without caring about the content
          length. This is safe because body in HTTP/2 is always
          segmented thanks to its framing layer. Meanwhile, we have to
          call Curl_read to ensure that http2_handle_stream_close is
          called when we read all incoming bytes for a particular
          stream. */
       !is_http2 &&
#endif
       k->size != -1 && !k->header) {
      /* make sure we don't read too much */
      curl_off_t totalleft = k->size - k->bytecount;
      if(totalleft < (curl_off_t)bytestoread)
        bytestoread = (size_t)totalleft;
    }

    if(bytestoread) {
      /* receive data from the network! */
      result = Curl_read(data, conn->sockfd, buf, bytestoread, &nread);

      /* read would've blocked */
      if(CURLE_AGAIN == result)
        break; /* get out of loop */

      if(result>0)
        return result;
    }
    else {
      /* read nothing but since we wanted nothing we consider this an OK
         situation to proceed from */
      DEBUGF(infof(data, "readwrite_data: we're done!"));
      nread = 0;
    }

    if(!k->bytecount) {
      Curl_pgrsTime(data, TIMER_STARTTRANSFER);
      if(k->exp100 > EXP100_SEND_DATA)
        /* set time stamp to compare with when waiting for the 100 */
        k->start100 = Curl_now();
    }

    *didwhat |= KEEP_RECV;
    /* indicates data of zero size, i.e. empty file */
    is_empty_data = ((nread == 0) && (k->bodywrites == 0)) ? TRUE : FALSE;

    if(0 < nread || is_empty_data) {
      buf[nread] = 0;
    }
    else {
      /* if we receive 0 or less here, either the http2 stream is closed or the
         server closed the connection and we bail out from this! */
#ifdef USE_NGHTTP2
      if(is_http2 && !nread)
        DEBUGF(infof(data, "nread == 0, stream closed, bailing"));
      else
#endif
        DEBUGF(infof(data, "nread <= 0, server closed connection, bailing"));
      k->keepon &= ~KEEP_RECV;
      break;
    }

    /* Default buffer to use when we write the buffer, it may be changed
       in the flow below before the actual storing is done. */
    k->str = buf;

    if(conn->handler->readwrite) {
      result = conn->handler->readwrite(data, conn, &nread, &readmore);
      if(result)
        return result;
      if(readmore)
        break;
    }

#ifndef CURL_DISABLE_HTTP
    /* Since this is a two-state thing, we check if we are parsing
       headers at the moment or not. */
    if(k->header) {
      /* we are in parse-the-header-mode */
      bool stop_reading = FALSE;
      result = Curl_http_readwrite_headers(data, conn, &nread, &stop_reading);
      if(result)
        return result;

      if(conn->handler->readwrite &&
         (k->maxdownload <= 0 && nread > 0)) {
        result = conn->handler->readwrite(data, conn, &nread, &readmore);
        if(result)
          return result;
        if(readmore)
          break;
      }

      if(stop_reading) {
        /* We've stopped dealing with input, get out of the do-while loop */

        if(nread > 0) {
          infof(data,
                "Excess found:"
                " excess = %zd"
                " url = %s (zero-length body)",
                nread, data->state.up.path);
        }

        break;
      }
    }
#endif /* CURL_DISABLE_HTTP */


    /* This is not an 'else if' since it may be a rest from the header
       parsing, where the beginning of the buffer is headers and the end
       is non-headers. */
    if(!k->header && (nread > 0 || is_empty_data)) {

      if(data->set.opt_no_body) {
        /* data arrives although we want none, bail out */
        streamclose(conn, "ignoring body");
        *done = TRUE;
        return CURLE_WEIRD_SERVER_REPLY;
      }

#ifndef CURL_DISABLE_HTTP
      if(0 == k->bodywrites && !is_empty_data) {
        /* These checks are only made the first time we are about to
           write a piece of the body */
        if(conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_RTSP)) {
          /* HTTP-only checks */
          result = Curl_http_firstwrite(data, conn, done);
          if(result || *done)
            return result;
        }
      } /* this is the first time we write a body part */
#endif /* CURL_DISABLE_HTTP */

      k->bodywrites++;

      /* pass data to the debug function before it gets "dechunked" */
      if(data->set.verbose) {
        if(k->badheader) {
          Curl_debug(data, CURLINFO_DATA_IN,
                     Curl_dyn_ptr(&data->state.headerb),
                     Curl_dyn_len(&data->state.headerb));
          if(k->badheader == HEADER_PARTHEADER)
            Curl_debug(data, CURLINFO_DATA_IN,
                       k->str, (size_t)nread);
        }
        else
          Curl_debug(data, CURLINFO_DATA_IN,
                     k->str, (size_t)nread);
      }

#ifndef CURL_DISABLE_HTTP
      if(k->chunk) {
        /*
         * Here comes a chunked transfer flying and we need to decode this
         * properly.  While the name says read, this function both reads
         * and writes away the data. The returned 'nread' holds the number
         * of actual data it wrote to the client.
         */
        CURLcode extra;
        CHUNKcode res =
          Curl_httpchunk_read(data, k->str, nread, &nread, &extra);

        if(CHUNKE_OK < res) {
          if(CHUNKE_PASSTHRU_ERROR == res) {
            failf(data, "Failed reading the chunked-encoded stream");
            return extra;
          }
          failf(data, "%s in chunked-encoding", Curl_chunked_strerror(res));
          return CURLE_RECV_ERROR;
        }
        if(CHUNKE_STOP == res) {
          /* we're done reading chunks! */
          k->keepon &= ~KEEP_RECV; /* read no more */

          /* N number of bytes at the end of the str buffer that weren't
             written to the client. */
          if(conn->chunk.datasize) {
            infof(data, "Leftovers after chunking: % "
                  CURL_FORMAT_CURL_OFF_T "u bytes",
                  conn->chunk.datasize);
          }
        }
        /* If it returned OK, we just keep going */
      }
#endif   /* CURL_DISABLE_HTTP */

      /* Account for body content stored in the header buffer */
      if((k->badheader == HEADER_PARTHEADER) && !k->ignorebody) {
        size_t headlen = Curl_dyn_len(&data->state.headerb);
        DEBUGF(infof(data, "Increasing bytecount by %zu", headlen));
        k->bytecount += headlen;
      }

      if((-1 != k->maxdownload) &&
         (k->bytecount + nread >= k->maxdownload)) {

        excess = (size_t)(k->bytecount + nread - k->maxdownload);
        if(excess > 0 && !k->ignorebody) {
          infof(data,
                "Excess found in a read:"
                " excess = %zu"
                ", size = %" CURL_FORMAT_CURL_OFF_T
                ", maxdownload = %" CURL_FORMAT_CURL_OFF_T
                ", bytecount = %" CURL_FORMAT_CURL_OFF_T,
                excess, k->size, k->maxdownload, k->bytecount);
          connclose(conn, "excess found in a read");
        }

        nread = (ssize_t) (k->maxdownload - k->bytecount);
        if(nread < 0) /* this should be unusual */
          nread = 0;

        k->keepon &= ~KEEP_RECV; /* we're done reading */
      }

      k->bytecount += nread;

      Curl_pgrsSetDownloadCounter(data, k->bytecount);

      if(!k->chunk && (nread || k->badheader || is_empty_data)) {
        /* If this is chunky transfer, it was already written */

        if(k->badheader && !k->ignorebody) {
          /* we parsed a piece of data wrongly assuming it was a header
             and now we output it as body instead */
          size_t headlen = Curl_dyn_len(&data->state.headerb);

          /* Don't let excess data pollute body writes */
          if(k->maxdownload == -1 || (curl_off_t)headlen <= k->maxdownload)
            result = Curl_client_write(data, CLIENTWRITE_BODY,
                                       Curl_dyn_ptr(&data->state.headerb),
                                       headlen);
          else
            result = Curl_client_write(data, CLIENTWRITE_BODY,
                                       Curl_dyn_ptr(&data->state.headerb),
                                       (size_t)k->maxdownload);

          if(result)
            return result;
        }
        if(k->badheader < HEADER_ALLBAD) {
          /* This switch handles various content encodings. If there's an
             error here, be sure to check over the almost identical code
             in http_chunks.c.
             Make sure that ALL_CONTENT_ENCODINGS contains all the
             encodings handled here. */
          if(data->set.http_ce_skip || !k->writer_stack) {
            if(!k->ignorebody && nread) {
#ifndef CURL_DISABLE_POP3
              if(conn->handler->protocol & PROTO_FAMILY_POP3)
                result = Curl_pop3_write(data, k->str, nread);
              else
#endif /* CURL_DISABLE_POP3 */
                result = Curl_client_write(data, CLIENTWRITE_BODY, k->str,
                                           nread);
            }
          }
          else if(!k->ignorebody && nread)
            result = Curl_unencode_write(data, k->writer_stack, k->str, nread);
        }
        k->badheader = HEADER_NORMAL; /* taken care of now */

        if(result)
          return result;
      }

    } /* if(!header and data to read) */

    if(conn->handler->readwrite && excess) {
      /* Parse the excess data */
      k->str += nread;

      if(&k->str[excess] > &buf[data->set.buffer_size]) {
        /* the excess amount was too excessive(!), make sure
           it doesn't read out of buffer */
        excess = &buf[data->set.buffer_size] - k->str;
      }
      nread = (ssize_t)excess;

      result = conn->handler->readwrite(data, conn, &nread, &readmore);
      if(result)
        return result;

      if(readmore)
        k->keepon |= KEEP_RECV; /* we're not done reading */
      break;
    }

    if(is_empty_data) {
      /* if we received nothing, the server closed the connection and we
         are done */
      k->keepon &= ~KEEP_RECV;
    }

    if(k->keepon & KEEP_RECV_PAUSE) {
      /* this is a paused transfer */
      break;
    }

  } while(data_pending(data) && maxloops--);

  if(maxloops <= 0) {
    /* we mark it as read-again-please */
    conn->cselect_bits = CURL_CSELECT_IN;
    *comeback = TRUE;
  }

  if(((k->keepon & (KEEP_RECV|KEEP_SEND)) == KEEP_SEND) &&
     conn->bits.close) {
    /* When we've read the entire thing and the close bit is set, the server
       may now close the connection. If there's now any kind of sending going
       on from our side, we need to stop that immediately. */
    infof(data, "we are done reading and this is set to close, stop send");
    k->keepon &= ~KEEP_SEND; /* no writing anymore either */
  }

  return CURLE_OK;
}


// Source: transfer.c
// Lines 559-906
