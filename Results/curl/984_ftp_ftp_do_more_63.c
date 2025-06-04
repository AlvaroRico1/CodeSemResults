static CURLcode ftp_do_more(struct Curl_easy *data, int *completep)
{
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  CURLcode result = CURLE_OK;
  bool connected = FALSE;
  bool complete = FALSE;

  /* the ftp struct is inited in ftp_connect() */
  struct FTP *ftp = data->req.p.ftp;

  /* if the second connection isn't done yet, wait for it */
  if(!conn->bits.tcpconnect[SECONDARYSOCKET]) {
    if(Curl_connect_ongoing(conn)) {
      /* As we're in TUNNEL_CONNECT state now, we know the proxy name and port
         aren't used so we blank their arguments. */
      result = Curl_proxyCONNECT(data, SECONDARYSOCKET, NULL, 0);

      return result;
    }

    result = Curl_is_connected(data, conn, SECONDARYSOCKET, &connected);

    /* Ready to do more? */
    if(connected) {
      DEBUGF(infof(data, "DO-MORE connected phase starts"));
    }
    else {
      if(result && (ftpc->count1 == 0)) {
        *completep = -1; /* go back to DOING please */
        /* this is a EPSV connect failing, try PASV instead */
        return ftp_epsv_disable(data, conn);
      }
      return result;
    }
  }

#ifndef CURL_DISABLE_PROXY
  result = Curl_proxy_connect(data, SECONDARYSOCKET);
  if(result)
    return result;

  if(CONNECT_SECONDARYSOCKET_PROXY_SSL())
    return result;

  if(conn->bits.tunnel_proxy && conn->bits.httpproxy &&
     Curl_connect_ongoing(conn))
    return result;
#endif

  if(ftpc->state) {
    /* already in a state so skip the initial commands.
       They are only done to kickstart the do_more state */
    result = ftp_multi_statemach(data, &complete);

    *completep = (int)complete;

    /* if we got an error or if we don't wait for a data connection return
       immediately */
    if(result || !ftpc->wait_data_conn)
      return result;

    /* if we reach the end of the FTP state machine here, *complete will be
       TRUE but so is ftpc->wait_data_conn, which says we need to wait for the
       data connection and therefore we're not actually complete */
    *completep = 0;
  }

  if(ftp->transfer <= PPTRANSFER_INFO) {
    /* a transfer is about to take place, or if not a file name was given
       so we'll do a SIZE on it later and then we need the right TYPE first */

    if(ftpc->wait_data_conn == TRUE) {
      bool serv_conned;

      result = ReceivedServerConnect(data, &serv_conned);
      if(result)
        return result; /* Failed to accept data connection */

      if(serv_conned) {
        /* It looks data connection is established */
        result = AcceptServerConnect(data);
        ftpc->wait_data_conn = FALSE;
        if(!result)
          result = InitiateTransfer(data);

        if(result)
          return result;

        *completep = 1; /* this state is now complete when the server has
                           connected back to us */
      }
    }
    else if(data->set.upload) {
      result = ftp_nb_type(data, conn, data->state.prefer_ascii,
                           FTP_STOR_TYPE);
      if(result)
        return result;

      result = ftp_multi_statemach(data, &complete);
      if(ftpc->wait_data_conn)
        /* if we reach the end of the FTP state machine here, *complete will be
           TRUE but so is ftpc->wait_data_conn, which says we need to wait for
           the data connection and therefore we're not actually complete */
        *completep = 0;
      else
        *completep = (int)complete;
    }
    else {
      /* download */
      ftp->downloadsize = -1; /* unknown as of yet */

      result = Curl_range(data);

      if(result == CURLE_OK && data->req.maxdownload >= 0) {
        /* Don't check for successful transfer */
        ftpc->dont_check = TRUE;
      }

      if(result)
        ;
      else if(data->state.list_only || !ftpc->file) {
        /* The specified path ends with a slash, and therefore we think this
           is a directory that is requested, use LIST. But before that we
           need to set ASCII transfer mode. */

        /* But only if a body transfer was requested. */
        if(ftp->transfer == PPTRANSFER_BODY) {
          result = ftp_nb_type(data, conn, TRUE, FTP_LIST_TYPE);
          if(result)
            return result;
        }
        /* otherwise just fall through */
      }
      else {
        result = ftp_nb_type(data, conn, data->state.prefer_ascii,
                             FTP_RETR_TYPE);
        if(result)
          return result;
      }

      result = ftp_multi_statemach(data, &complete);
      *completep = (int)complete;
    }
    return result;
  }

  /* no data to transfer */
  Curl_setup_transfer(data, -1, -1, FALSE, -1);

  if(!ftpc->wait_data_conn) {
    /* no waiting for the data connection so this is now complete */
    *completep = 1;
    DEBUGF(infof(data, "DO-MORE phase ends with %d", (int)result));
  }

  return result;
}


// Source: ftp.c
// Lines 3553-3710
