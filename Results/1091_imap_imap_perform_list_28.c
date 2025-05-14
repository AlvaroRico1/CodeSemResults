// Source: curl/lib/imap.c
// Lines 622-625
static CURLcode imap_perform_list(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct IMAP *imap = data->req.p.imap;

  if(imap->custom)
    /* Send the custom request */
    result = imap_sendf(data, conn, "%s%s", imap->custom,
                        imap->custom_params ? imap->custom_params : "");
  else {
    /* Make sure the mailbox is in the correct atom format if necessary */
    char *mailbox = imap->mailbox ? imap_atom(imap->mailbox, true)
                                  : strdup("");
    if(!mailbox)
      return CURLE_OUT_OF_MEMORY;

    /* Send the LIST command */
    result = imap_sendf(data, conn, "LIST \"%s\" *", mailbox);

    free(mailbox);
  }

  if(!result)
    state(data, IMAP_LIST);

  return result;
}
