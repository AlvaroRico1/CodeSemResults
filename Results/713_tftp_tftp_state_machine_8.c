// Source: curl/lib/tftp.c
// Lines 901-905
static CURLcode tftp_state_machine(struct tftp_state_data *state,
                                   tftp_event_t event)
{
  CURLcode result = CURLE_OK;
  struct Curl_easy *data = state->data;

  switch(state->state) {
  case TFTP_STATE_START:
    DEBUGF(infof(data, "TFTP_STATE_START"));
    result = tftp_send_first(state, event);
    break;
  case TFTP_STATE_RX:
    DEBUGF(infof(data, "TFTP_STATE_RX"));
    result = tftp_rx(state, event);
    break;
  case TFTP_STATE_TX:
    DEBUGF(infof(data, "TFTP_STATE_TX"));
    result = tftp_tx(state, event);
    break;
  case TFTP_STATE_FIN:
    infof(data, "%s", "TFTP finished");
    break;
  default:
    DEBUGF(infof(data, "STATE: %d", state->state));
    failf(data, "%s", "Internal state machine error");
    result = CURLE_TFTP_ILLEGAL;
    break;
  }

  return result;
}
