static CURLcode imap_state_servergreet_resp(struct Curl_easy *data,
                                            int imapcode,
                                            imapstate instate)
{
  struct connectdata *conn = data->conn;
  (void)instate; /* no use for this yet */

  if(imapcode == IMAP_RESP_PREAUTH) {
    /* PREAUTH */
    struct imap_conn *imapc = &conn->proto.imapc;
    imapc->preauth = TRUE;
    infof(data, "PREAUTH connection, already authenticated!");
  }
  else if(imapcode != IMAP_RESP_OK) {
    failf(data, "Got unexpected imap-server response");
    return CURLE_WEIRD_SERVER_REPLY;
  }

  return imap_perform_capability(data, conn);
}


// Source: imap.c
// Lines 852-871
