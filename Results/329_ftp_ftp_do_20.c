// Source: curl/lib/ftp.c
// Lines 3999-4002
static CURLcode ftp_do(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;

  *done = FALSE; /* default to false */
  ftpc->wait_data_conn = FALSE; /* default to no such wait */

  if(data->state.wildcardmatch) {
    result = wc_statemach(data);
    if(data->wildcard.state == CURLWC_SKIP ||
      data->wildcard.state == CURLWC_DONE) {
      /* do not call ftp_regular_transfer */
      return CURLE_OK;
    }
    if(result) /* error, loop or skipping the file */
      return result;
  }
  else { /* no wildcard FSM needed */
    result = ftp_parse_url_path(data);
    if(result)
      return result;
  }

  result = ftp_regular_transfer(data, done);

  return result;
}
