static CURLcode imap_state_select_resp(struct Curl_easy *data, int imapcode,
                                       imapstate instate)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct IMAP *imap = data->req.p.imap;
  struct imap_conn *imapc = &conn->proto.imapc;
  const char *line = data->state.buffer;

  (void)instate; /* no use for this yet */

  if(imapcode == '*') {
    /* See if this is an UIDVALIDITY response */
    char tmp[20];
    if(sscanf(line + 2, "OK [UIDVALIDITY %19[0123456789]]", tmp) == 1) {
      Curl_safefree(imapc->mailbox_uidvalidity);
      imapc->mailbox_uidvalidity = strdup(tmp);
    }
  }
  else if(imapcode == IMAP_RESP_OK) {
    /* Check if the UIDVALIDITY has been specified and matches */
    if(imap->uidvalidity && imapc->mailbox_uidvalidity &&
       !strcasecompare(imap->uidvalidity, imapc->mailbox_uidvalidity)) {
      failf(data, "Mailbox UIDVALIDITY has changed");
      result = CURLE_REMOTE_FILE_NOT_FOUND;
    }
    else {
      /* Note the currently opened mailbox on this connection */
      imapc->mailbox = strdup(imap->mailbox);

      if(imap->custom)
        result = imap_perform_list(data);
      else if(imap->query)
        result = imap_perform_search(data, conn);
      else
        result = imap_perform_fetch(data, conn);
    }
  }
  else {
    failf(data, "Select failed");
    result = CURLE_LOGIN_DENIED;
  }

  return result;
}


// Source: imap.c
// Lines 1064-1108
