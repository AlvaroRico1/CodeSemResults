static CURLcode tftp_do(struct Curl_easy *data, bool *done)
{
  struct tftp_state_data *state;
  CURLcode result;
  struct connectdata *conn = data->conn;

  *done = FALSE;

  if(!conn->proto.tftpc) {
    result = tftp_connect(data, done);
    if(result)
      return result;
  }

  state = conn->proto.tftpc;
  if(!state)
    return CURLE_TFTP_ILLEGAL;

  result = tftp_perform(data, done);

  /* If tftp_perform() returned an error, use that for return code. If it
     was OK, see if tftp_translate_code() has an error. */
  if(!result)
    /* If we have encountered an internal tftp error, translate it. */
    result = tftp_translate_code(state->error);

  return result;
}


// Source: tftp.c
// Lines 1344-1371
