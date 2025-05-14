// Source: curl/lib/ftp.c
// Lines 4258-4260
static CURLcode ftp_dophase_done(struct Curl_easy *data, bool connected)
{
  struct connectdata *conn = data->conn;
  struct FTP *ftp = data->req.p.ftp;
  struct ftp_conn *ftpc = &conn->proto.ftpc;

  if(connected) {
    int completed;
    CURLcode result = ftp_do_more(data, &completed);

    if(result) {
      close_secondarysocket(data, conn);
      return result;
    }
  }

  if(ftp->transfer != PPTRANSFER_BODY)
    /* no data to transfer */
    Curl_setup_transfer(data, -1, -1, FALSE, -1);
  else if(!connected)
    /* since we didn't connect now, we want do_more to get called */
    conn->bits.do_more = TRUE;

  ftpc->ctl_valid = TRUE; /* seems good */

  return CURLE_OK;
}
