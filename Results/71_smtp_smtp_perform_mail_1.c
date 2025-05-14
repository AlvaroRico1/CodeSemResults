// Source: curl/lib/smtp.c
// Lines 573-579
static CURLcode smtp_perform_mail(struct Curl_easy *data)
{
  char *from = NULL;
  char *auth = NULL;
  char *size = NULL;
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  /* We notify the server we are sending UTF-8 data if a) it supports the
     SMTPUTF8 extension and b) The mailbox contains UTF-8 charaacters, in
     either the local address or host name parts. This is regardless of
     whether the host name is encoded using IDN ACE */
  bool utf8 = FALSE;

  /* Calculate the FROM parameter */
  if(data->set.str[STRING_MAIL_FROM]) {
    char *address = NULL;
    struct hostname host = { NULL, NULL, NULL, NULL };

    /* Parse the FROM mailbox into the local address and host name parts,
       converting the host name to an IDN A-label if necessary */
    result = smtp_parse_address(data, data->set.str[STRING_MAIL_FROM],
                                &address, &host);
    if(result)
      return result;

    /* Establish whether we should report SMTPUTF8 to the server for this
       mailbox as per RFC-6531 sect. 3.1 point 4 and sect. 3.4 */
    utf8 = (conn->proto.smtpc.utf8_supported) &&
           ((host.encalloc) || (!Curl_is_ASCII_name(address)) ||
            (!Curl_is_ASCII_name(host.name)));

    if(host.name) {
      from = aprintf("<%s@%s>", address, host.name);

      Curl_free_idnconverted_hostname(&host);
    }
    else
      /* An invalid mailbox was provided but we'll simply let the server worry
         about that and reply with a 501 error */
      from = aprintf("<%s>", address);

    free(address);
  }
  else
    /* Null reverse-path, RFC-5321, sect. 3.6.3 */
    from = strdup("<>");

  if(!from)
    return CURLE_OUT_OF_MEMORY;

  /* Calculate the optional AUTH parameter */
  if(data->set.str[STRING_MAIL_AUTH] && conn->proto.smtpc.sasl.authused) {
    if(data->set.str[STRING_MAIL_AUTH][0] != '\0') {
      char *address = NULL;
      struct hostname host = { NULL, NULL, NULL, NULL };

      /* Parse the AUTH mailbox into the local address and host name parts,
         converting the host name to an IDN A-label if necessary */
      result = smtp_parse_address(data, data->set.str[STRING_MAIL_AUTH],
                                  &address, &host);
      if(result) {
        free(from);
        return result;
      }

      /* Establish whether we should report SMTPUTF8 to the server for this
         mailbox as per RFC-6531 sect. 3.1 point 4 and sect. 3.4 */
      if((!utf8) && (conn->proto.smtpc.utf8_supported) &&
         ((host.encalloc) || (!Curl_is_ASCII_name(address)) ||
          (!Curl_is_ASCII_name(host.name))))
        utf8 = TRUE;

      if(host.name) {
        auth = aprintf("<%s@%s>", address, host.name);

        Curl_free_idnconverted_hostname(&host);
      }
      else
        /* An invalid mailbox was provided but we'll simply let the server
           worry about it */
        auth = aprintf("<%s>", address);

      free(address);
    }
    else
      /* Empty AUTH, RFC-2554, sect. 5 */
      auth = strdup("<>");

    if(!auth) {
      free(from);

      return CURLE_OUT_OF_MEMORY;
    }
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

    if(result) {
      free(from);
      free(auth);

      return result;
    }

    data->state.infilesize = Curl_mime_size(&data->set.mimepost);

    /* Read from mime structure. */
    data->state.fread_func = (curl_read_callback) Curl_mime_read;
    data->state.in = (void *) &data->set.mimepost;
  }

  /* Calculate the optional SIZE parameter */
  if(conn->proto.smtpc.size_supported && data->state.infilesize > 0) {
    size = aprintf("%" CURL_FORMAT_CURL_OFF_T, data->state.infilesize);

    if(!size) {
      free(from);
      free(auth);

      return CURLE_OUT_OF_MEMORY;
    }
  }

  /* If the mailboxes in the FROM and AUTH parameters don't include a UTF-8
     based address then quickly scan through the recipient list and check if
     any there do, as we need to correctly identify our support for SMTPUTF8
     in the envelope, as per RFC-6531 sect. 3.4 */
  if(conn->proto.smtpc.utf8_supported && !utf8) {
    struct SMTP *smtp = data->req.p.smtp;
    struct curl_slist *rcpt = smtp->rcpt;

    while(rcpt && !utf8) {
      /* Does the host name contain non-ASCII characters? */
      if(!Curl_is_ASCII_name(rcpt->data))
        utf8 = TRUE;

      rcpt = rcpt->next;
    }
  }

  /* Send the MAIL command */
  result = Curl_pp_sendf(data, &conn->proto.smtpc.pp,
                         "MAIL FROM:%s%s%s%s%s%s",
                         from,                 /* Mandatory                 */
                         auth ? " AUTH=" : "", /* Optional on AUTH support  */
                         auth ? auth : "",     /*                           */
                         size ? " SIZE=" : "", /* Optional on SIZE support  */
                         size ? size : "",     /*                           */
                         utf8 ? " SMTPUTF8"    /* Internationalised mailbox */
                               : "");          /* included in our envelope  */

  free(from);
  free(auth);
  free(size);

  if(!result)
    state(data, SMTP_MAIL);

  return result;
}
