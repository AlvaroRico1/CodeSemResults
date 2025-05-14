// Source: curl/lib/ftp.c
// Lines 2389-2393
static CURLcode ftp_state_stor_resp(struct Curl_easy *data,
                                    int ftpcode, ftpstate instate)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  if(ftpcode >= 400) {
    failf(data, "Failed FTP upload: %0d", ftpcode);
    state(data, FTP_STOP);
    /* oops, we never close the sockets! */
    return CURLE_UPLOAD_FAILED;
  }

  conn->proto.ftpc.state_saved = instate;

  /* PORT means we are now awaiting the server to connect to us. */
  if(data->set.ftp_use_port) {
    bool connected;

    state(data, FTP_STOP); /* no longer in STOR state */

    result = AllowServerConnect(data, &connected);
    if(result)
      return result;

    if(!connected) {
      struct ftp_conn *ftpc = &conn->proto.ftpc;
      infof(data, "Data conn was not available immediately");
      ftpc->wait_data_conn = TRUE;
    }

    return CURLE_OK;
  }
  return InitiateTransfer(data);
}
