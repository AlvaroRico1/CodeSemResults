static CURLcode tftp_multi_statemach(struct Curl_easy *data, bool *done)
{
  tftp_event_t event;
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct tftp_state_data *state = conn->proto.tftpc;
  timediff_t timeout_ms = tftp_state_timeout(data, &event);

  *done = FALSE;

  if(timeout_ms < 0) {
    failf(data, "TFTP response timeout");
    return CURLE_OPERATION_TIMEDOUT;
  }
  if(event != TFTP_EVENT_NONE) {
    result = tftp_state_machine(state, event);
    if(result)
      return result;
    *done = (state->state == TFTP_STATE_FIN) ? TRUE : FALSE;
    if(*done)
      /* Tell curl we're done */
      Curl_setup_transfer(data, -1, -1, FALSE, -1);
  }
  else {
    /* no timeouts to handle, check our socket */
    int rc = SOCKET_READABLE(state->sockfd, 0);

    if(rc == -1) {
      /* bail out */
      int error = SOCKERRNO;
      char buffer[STRERROR_LEN];
      failf(data, "%s", Curl_strerror(error, buffer, sizeof(buffer)));
      state->event = TFTP_EVENT_ERROR;
    }
    else if(rc) {
      result = tftp_receive_packet(data);
      if(result)
        return result;
      result = tftp_state_machine(state, state->event);
      if(result)
        return result;
      *done = (state->state == TFTP_STATE_FIN) ? TRUE : FALSE;
      if(*done)
        /* Tell curl we're done */
        Curl_setup_transfer(data, -1, -1, FALSE, -1);
    }
    /* if rc == 0, then select() timed out */
  }

  return result;
}


// Source: tftp.c
// Lines 1226-1276
