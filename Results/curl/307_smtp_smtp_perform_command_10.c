static CURLcode smtp_perform_command(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct SMTP *smtp = data->req.p.smtp;

  if(smtp->rcpt) {
    /* We notify the server we are sending UTF-8 data if a) it supports the
       SMTPUTF8 extension and b) The mailbox contains UTF-8 charaacters, in
       either the local address or host name parts. This is regardless of
       whether the host name is encoded using IDN ACE */
    bool utf8 = FALSE;

    if((!smtp->custom) || (!smtp->custom[0])) {
      char *address = NULL;
      struct hostname host = { NULL, NULL, NULL, NULL };

      /* Parse the mailbox to verify into the local address and host name
         parts, converting the host name to an IDN A-label if necessary */
      result = smtp_parse_address(data, smtp->rcpt->data,
                                  &address, &host);
      if(result)
        return result;

      /* Establish whether we should report SMTPUTF8 to the server for this
         mailbox as per RFC-6531 sect. 3.1 point 6 */
      utf8 = (conn->proto.smtpc.utf8_supported) &&
             ((host.encalloc) || (!Curl_is_ASCII_name(address)) ||
              (!Curl_is_ASCII_name(host.name)));

      /* Send the VRFY command (Note: The host name part may be absent when the
         host is a local system) */
      result = Curl_pp_sendf(data, &conn->proto.smtpc.pp, "VRFY %s%s%s%s",
                             address,
                             host.name ? "@" : "",
                             host.name ? host.name : "",
                             utf8 ? " SMTPUTF8" : "");

      Curl_free_idnconverted_hostname(&host);
      free(address);
    }
    else {
      /* Establish whether we should report that we support SMTPUTF8 for EXPN
         commands to the server as per RFC-6531 sect. 3.1 point 6 */
      utf8 = (conn->proto.smtpc.utf8_supported) &&
             (!strcmp(smtp->custom, "EXPN"));

      /* Send the custom recipient based command such as the EXPN command */
      result = Curl_pp_sendf(data, &conn->proto.smtpc.pp,
                             "%s %s%s", smtp->custom,
                             smtp->rcpt->data,
                             utf8 ? " SMTPUTF8" : "");
    }
  }
  else
    /* Send the non-recipient based command such as HELP */
    result = Curl_pp_sendf(data, &conn->proto.smtpc.pp, "%s",
                           smtp->custom && smtp->custom[0] != '\0' ?
                           smtp->custom : "HELP");

  if(!result)
    state(data, SMTP_COMMAND);

  return result;
}


// Source: smtp.c
// Lines 501-565
