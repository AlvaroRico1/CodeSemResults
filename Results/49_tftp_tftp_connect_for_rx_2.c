static CURLcode tftp_connect_for_rx(struct tftp_state_data *state,
                                    tftp_event_t event)
{
  CURLcode result;
#ifndef CURL_DISABLE_VERBOSE_STRINGS
  struct Curl_easy *data = state->data;

  infof(data, "%s", "Connected for receive");
#endif
  state->state = TFTP_STATE_RX;
  result = tftp_set_timeouts(state);
  if(result)
    return result;
  return tftp_rx(state, event);
}


// Source: tftp.c
// Lines 409-423
