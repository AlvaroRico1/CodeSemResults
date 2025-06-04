static timediff_t tftp_state_timeout(struct Curl_easy *data,
                                     tftp_event_t *event)
{
  time_t current;
  struct connectdata *conn = data->conn;
  struct tftp_state_data *state = conn->proto.tftpc;
  timediff_t timeout_ms;

  if(event)
    *event = TFTP_EVENT_NONE;

  timeout_ms = Curl_timeleft(state->data, NULL,
                             (state->state == TFTP_STATE_START));
  if(timeout_ms < 0) {
    state->error = TFTP_ERR_TIMEOUT;
    state->state = TFTP_STATE_FIN;
    return 0;
  }
  time(&current);
  if(current > state->rx_time + state->retry_time) {
    if(event)
      *event = TFTP_EVENT_TIMEOUT;
    time(&state->rx_time); /* update even though we received nothing */
  }

  return timeout_ms;
}


// Source: tftp.c
// Lines 1191-1217
