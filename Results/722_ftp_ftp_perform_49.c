CURLcode ftp_perform(struct Curl_easy *data,
                     bool *connected,  /* connect status after PASV / PORT */
                     bool *dophase_done)
{
  /* this is FTP and no proxy */
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  DEBUGF(infof(data, "DO phase starts"));

  if(data->set.opt_no_body) {
    /* requested no body means no transfer... */
    struct FTP *ftp = data->req.p.ftp;
    ftp->transfer = PPTRANSFER_INFO;
  }

  *dophase_done = FALSE; /* not done yet */

  /* start the first command in the DO phase */
  result = ftp_state_quote(data, TRUE, FTP_QUOTE);
  if(result)
    return result;

  /* run the state-machine */
  result = ftp_multi_statemach(data, dophase_done);

  *connected = conn->bits.tcpconnect[SECONDARYSOCKET];

  infof(data, "ftp_perform ends with SECONDARY: %d", *connected);

  if(*dophase_done)
    DEBUGF(infof(data, "DO phase is complete1"));

  return result;
}


// Source: ftp.c
// Lines 3723-3757
