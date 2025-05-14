// Source: curl/lib/tftp.c
// Lines 393-398
static CURLcode tftp_connect_for_tx(struct tftp_state_data *state,
                                    tftp_event_t event)
{
  CURLcode result;
#ifndef CURL_DISABLE_VERBOSE_STRINGS
  struct Curl_easy *data = state->data;

  infof(data, "%s", "Connected for transmit");
#endif
  state->state = TFTP_STATE_TX;
  result = tftp_set_timeouts(state);
  if(result)
    return result;
  return tftp_tx(state, event);
}
