static CURLcode smtp_perform_rcpt_to(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct SMTP *smtp = data->req.p.smtp;
  char *address = NULL;
  struct hostname host = { NULL, NULL, NULL, NULL };

  /* Parse the recipient mailbox into the local address and host name parts,
     converting the host name to an IDN A-label if necessary */
  result = smtp_parse_address(data, smtp->rcpt->data,
                              &address, &host);
  if(result)
    return result;

  /* Send the RCPT TO command */
  if(host.name)
    result = Curl_pp_sendf(data, &conn->proto.smtpc.pp, "RCPT TO:<%s@%s>",
                           address, host.name);
  else
    /* An invalid mailbox was provided but we'll simply let the server worry
       about that and reply with a 501 error */
    result = Curl_pp_sendf(data, &conn->proto.smtpc.pp, "RCPT TO:<%s>",
                           address);

  Curl_free_idnconverted_hostname(&host);
  free(address);

  if(!result)
    state(data, SMTP_RCPT);

  return result;
}


// Source: smtp.c
// Lines 759-791
