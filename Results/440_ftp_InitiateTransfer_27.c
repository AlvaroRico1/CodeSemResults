// Source: curl/lib/ftp.c
// Lines 439-442
static CURLcode InitiateTransfer(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  if(conn->bits.ftp_use_data_ssl) {
    /* since we only have a plaintext TCP connection here, we must now
     * do the TLS stuff */
    infof(data, "Doing the SSL/TLS handshake on the data stream");
    result = Curl_ssl_connect(data, conn, SECONDARYSOCKET);
    if(result)
      return result;
  }

  if(conn->proto.ftpc.state_saved == FTP_STOR) {
    /* When we know we're uploading a specified file, we can get the file
       size prior to the actual upload. */
    Curl_pgrsSetUploadSize(data, data->state.infilesize);

    /* set the SO_SNDBUF for the secondary socket for those who need it */
    Curl_sndbufset(conn->sock[SECONDARYSOCKET]);

    Curl_setup_transfer(data, -1, -1, FALSE, SECONDARYSOCKET);
  }
  else {
    /* FTP download: */
    Curl_setup_transfer(data, SECONDARYSOCKET,
                        conn->proto.ftpc.retr_size_saved, FALSE, -1);
  }

  conn->proto.ftpc.pp.pending_resp = TRUE; /* expect server response */
  state(data, FTP_STOP);

  return CURLE_OK;
}
