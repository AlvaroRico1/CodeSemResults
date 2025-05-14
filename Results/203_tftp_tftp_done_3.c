// Source: curl/lib/tftp.c
// Lines 1058-1062
static CURLcode tftp_done(struct Curl_easy *data, CURLcode status,
                          bool premature)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct tftp_state_data *state = conn->proto.tftpc;

  (void)status; /* unused */
  (void)premature; /* not used */

  if(Curl_pgrsDone(data))
    return CURLE_ABORTED_BY_CALLBACK;

  /* If we have encountered an error */
  if(state)
    result = tftp_translate_code(state->error);

  return result;
}
