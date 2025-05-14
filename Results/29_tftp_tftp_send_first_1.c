// Source: curl/lib/tftp.c
// Lines 425-432
static CURLcode tftp_send_first(struct tftp_state_data *state,
                                tftp_event_t event)
{
  size_t sbytes;
  ssize_t senddata;
  const char *mode = "octet";
  char *filename;
  struct Curl_easy *data = state->data;
  CURLcode result = CURLE_OK;

  /* Set ascii mode if -B flag was used */
  if(data->state.prefer_ascii)
    mode = "netascii";

  switch(event) {

  case TFTP_EVENT_INIT:    /* Send the first packet out */
  case TFTP_EVENT_TIMEOUT: /* Resend the first packet out */
    /* Increment the retry counter, quit if over the limit */
    state->retries++;
    if(state->retries>state->retry_max) {
      state->error = TFTP_ERR_NORESPONSE;
      state->state = TFTP_STATE_FIN;
      return result;
    }

    if(data->set.upload) {
      /* If we are uploading, send an WRQ */
      setpacketevent(&state->spacket, TFTP_EVENT_WRQ);
      state->data->req.upload_fromhere =
        (char *)state->spacket.data + 4;
      if(data->state.infilesize != -1)
        Curl_pgrsSetUploadSize(data, data->state.infilesize);
    }
    else {
      /* If we are downloading, send an RRQ */
      setpacketevent(&state->spacket, TFTP_EVENT_RRQ);
    }
    /* As RFC3617 describes the separator slash is not actually part of the
       file name so we skip the always-present first letter of the path
       string. */
    result = Curl_urldecode(data, &state->data->state.up.path[1], 0,
                            &filename, NULL, REJECT_ZERO);
    if(result)
      return result;

    if(strlen(filename) > (state->blksize - strlen(mode) - 4)) {
      failf(data, "TFTP file name too long");
      free(filename);
      return CURLE_TFTP_ILLEGAL; /* too long file name field */
    }

    msnprintf((char *)state->spacket.data + 2,
              state->blksize,
              "%s%c%s%c", filename, '\0',  mode, '\0');
    sbytes = 4 + strlen(filename) + strlen(mode);

    /* optional addition of TFTP options */
    if(!data->set.tftp_no_options) {
      char buf[64];
      /* add tsize option */
      if(data->set.upload && (data->state.infilesize != -1))
        msnprintf(buf, sizeof(buf), "%" CURL_FORMAT_CURL_OFF_T,
                  data->state.infilesize);
      else
        strcpy(buf, "0"); /* the destination is large enough */

      result = tftp_option_add(state, &sbytes,
                               (char *)state->spacket.data + sbytes,
                               TFTP_OPTION_TSIZE);
      if(result == CURLE_OK)
        result = tftp_option_add(state, &sbytes,
                                 (char *)state->spacket.data + sbytes, buf);

      /* add blksize option */
      msnprintf(buf, sizeof(buf), "%d", state->requested_blksize);
      if(result == CURLE_OK)
        result = tftp_option_add(state, &sbytes,
                                 (char *)state->spacket.data + sbytes,
                                 TFTP_OPTION_BLKSIZE);
      if(result == CURLE_OK)
        result = tftp_option_add(state, &sbytes,
                                 (char *)state->spacket.data + sbytes, buf);

      /* add timeout option */
      msnprintf(buf, sizeof(buf), "%d", state->retry_time);
      if(result == CURLE_OK)
        result = tftp_option_add(state, &sbytes,
                                 (char *)state->spacket.data + sbytes,
                                 TFTP_OPTION_INTERVAL);
      if(result == CURLE_OK)
        result = tftp_option_add(state, &sbytes,
                                 (char *)state->spacket.data + sbytes, buf);

      if(result != CURLE_OK) {
        failf(data, "TFTP buffer too small for options");
        free(filename);
        return CURLE_TFTP_ILLEGAL;
      }
    }

    /* the typecase for the 3rd argument is mostly for systems that do
       not have a size_t argument, like older unixes that want an 'int' */
    senddata = sendto(state->sockfd, (void *)state->spacket.data,
                      (SEND_TYPE_ARG3)sbytes, 0,
                      data->conn->ip_addr->ai_addr,
                      data->conn->ip_addr->ai_addrlen);
    if(senddata != (ssize_t)sbytes) {
      char buffer[STRERROR_LEN];
      failf(data, "%s", Curl_strerror(SOCKERRNO, buffer, sizeof(buffer)));
    }
    free(filename);
    break;

  case TFTP_EVENT_OACK:
    if(data->set.upload) {
      result = tftp_connect_for_tx(state, event);
    }
    else {
      result = tftp_connect_for_rx(state, event);
    }
    break;

  case TFTP_EVENT_ACK: /* Connected for transmit */
    result = tftp_connect_for_tx(state, event);
    break;

  case TFTP_EVENT_DATA: /* Connected for receive */
    result = tftp_connect_for_rx(state, event);
    break;

  case TFTP_EVENT_ERROR:
    state->state = TFTP_STATE_FIN;
    break;

  default:
    failf(state->data, "tftp_send_first: internal error");
    break;
  }

  return result;
}
