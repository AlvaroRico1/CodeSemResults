static CURLcode imap_perform_append(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct IMAP *imap = data->req.p.imap;
  char *mailbox;

  /* Check we have a mailbox */
  if(!imap->mailbox) {
    failf(data, "Cannot APPEND without a mailbox.");
    return CURLE_URL_MALFORMAT;
  }

  /* Prepare the mime data if some. */
  if(data->set.mimepost.kind != MIMEKIND_NONE) {
    /* Use the whole structure as data. */
    data->set.mimepost.flags &= ~MIME_BODY_ONLY;

    /* Add external headers and mime version. */
    curl_mime_headers(&data->set.mimepost, data->set.headers, 0);
    result = Curl_mime_prepare_headers(&data->set.mimepost, NULL,
                                       NULL, MIMESTRATEGY_MAIL);

    if(!result)
      if(!Curl_checkheaders(data, "Mime-Version"))
        result = Curl_mime_add_header(&data->set.mimepost.curlheaders,
                                      "Mime-Version: 1.0");

    /* Make sure we will read the entire mime structure. */
    if(!result)
      result = Curl_mime_rewind(&data->set.mimepost);

    if(result)
      return result;

    data->state.infilesize = Curl_mime_size(&data->set.mimepost);

    /* Read from mime structure. */
    data->state.fread_func = (curl_read_callback) Curl_mime_read;
    data->state.in = (void *) &data->set.mimepost;
  }

  /* Check we know the size of the upload */
  if(data->state.infilesize < 0) {
    failf(data, "Cannot APPEND with unknown input file size");
    return CURLE_UPLOAD_FAILED;
  }

  /* Make sure the mailbox is in the correct atom format */
  mailbox = imap_atom(imap->mailbox, false);
  if(!mailbox)
    return CURLE_OUT_OF_MEMORY;

  /* Send the APPEND command */
  result = imap_sendf(data, conn,
                      "APPEND %s (\\Seen) {%" CURL_FORMAT_CURL_OFF_T "}",
                      mailbox, data->state.infilesize);

  free(mailbox);

  if(!result)
    state(data, IMAP_APPEND);

  return result;
}


// Source: imap.c
// Lines 740-804
