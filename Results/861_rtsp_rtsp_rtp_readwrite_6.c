// Source: curl/lib/rtsp.c
// Lines 607-611
static CURLcode rtsp_rtp_readwrite(struct Curl_easy *data,
                                   struct connectdata *conn,
                                   ssize_t *nread,
                                   bool *readmore) {
  struct SingleRequest *k = &data->req;
  struct rtsp_conn *rtspc = &(conn->proto.rtspc);

  char *rtp; /* moving pointer to rtp data */
  ssize_t rtp_dataleft; /* how much data left to parse in this round */
  char *scratch;
  CURLcode result;

  if(rtspc->rtp_buf) {
    /* There was some leftover data the last time. Merge buffers */
    char *newptr = Curl_saferealloc(rtspc->rtp_buf,
                                    rtspc->rtp_bufsize + *nread);
    if(!newptr) {
      rtspc->rtp_buf = NULL;
      rtspc->rtp_bufsize = 0;
      return CURLE_OUT_OF_MEMORY;
    }
    rtspc->rtp_buf = newptr;
    memcpy(rtspc->rtp_buf + rtspc->rtp_bufsize, k->str, *nread);
    rtspc->rtp_bufsize += *nread;
    rtp = rtspc->rtp_buf;
    rtp_dataleft = rtspc->rtp_bufsize;
  }
  else {
    /* Just parse the request buffer directly */
    rtp = k->str;
    rtp_dataleft = *nread;
  }

  while((rtp_dataleft > 0) &&
        (rtp[0] == '$')) {
    if(rtp_dataleft > 4) {
      int rtp_length;

      /* Parse the header */
      /* The channel identifier immediately follows and is 1 byte */
      rtspc->rtp_channel = RTP_PKT_CHANNEL(rtp);

      /* The length is two bytes */
      rtp_length = RTP_PKT_LENGTH(rtp);

      if(rtp_dataleft < rtp_length + 4) {
        /* Need more - incomplete payload*/
        *readmore = TRUE;
        break;
      }
      /* We have the full RTP interleaved packet
       * Write out the header including the leading '$' */
      DEBUGF(infof(data, "RTP write channel %d rtp_length %d",
             rtspc->rtp_channel, rtp_length));
      result = rtp_client_write(data, &rtp[0], rtp_length + 4);
      if(result) {
        failf(data, "Got an error writing an RTP packet");
        *readmore = FALSE;
        Curl_safefree(rtspc->rtp_buf);
        rtspc->rtp_buf = NULL;
        rtspc->rtp_bufsize = 0;
        return result;
      }

      /* Move forward in the buffer */
      rtp_dataleft -= rtp_length + 4;
      rtp += rtp_length + 4;

      if(data->set.rtspreq == RTSPREQ_RECEIVE) {
        /* If we are in a passive receive, give control back
         * to the app as often as we can.
         */
        k->keepon &= ~KEEP_RECV;
      }
    }
    else {
      /* Need more - incomplete header */
      *readmore = TRUE;
      break;
    }
  }

  if(rtp_dataleft && rtp[0] == '$') {
    DEBUGF(infof(data, "RTP Rewinding %zd %s", rtp_dataleft,
          *readmore ? "(READMORE)" : ""));

    /* Store the incomplete RTP packet for a "rewind" */
    scratch = malloc(rtp_dataleft);
    if(!scratch) {
      Curl_safefree(rtspc->rtp_buf);
      rtspc->rtp_buf = NULL;
      rtspc->rtp_bufsize = 0;
      return CURLE_OUT_OF_MEMORY;
    }
    memcpy(scratch, rtp, rtp_dataleft);
    Curl_safefree(rtspc->rtp_buf);
    rtspc->rtp_buf = scratch;
    rtspc->rtp_bufsize = rtp_dataleft;

    /* As far as the transfer is concerned, this data is consumed */
    *nread = 0;
    return CURLE_OK;
  }
  /* Fix up k->str to point just after the last RTP packet */
  k->str += *nread - rtp_dataleft;

  /* either all of the data has been read or...
   * rtp now points at the next byte to parse
   */
  if(rtp_dataleft > 0)
    DEBUGASSERT(k->str[0] == rtp[0]);

  DEBUGASSERT(rtp_dataleft <= *nread); /* sanity check */

  *nread = rtp_dataleft;

  /* If we get here, we have finished with the leftover/merge buffer */
  Curl_safefree(rtspc->rtp_buf);
  rtspc->rtp_buf = NULL;
  rtspc->rtp_bufsize = 0;

  return CURLE_OK;
}
