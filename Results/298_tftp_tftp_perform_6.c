// Source: curl/lib/tftp.c
// Lines 1312-1315
static CURLcode tftp_perform(struct Curl_easy *data, bool *dophase_done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct tftp_state_data *state = conn->proto.tftpc;

  *dophase_done = FALSE;

  result = tftp_state_machine(state, TFTP_EVENT_INIT);

  if((state->state == TFTP_STATE_FIN) || result)
    return result;

  tftp_multi_statemach(data, dophase_done);

  if(*dophase_done)
    DEBUGF(infof(data, "DO phase is complete"));

  return result;
}
