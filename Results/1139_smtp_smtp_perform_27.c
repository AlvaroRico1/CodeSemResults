static CURLcode smtp_perform(struct Curl_easy *data, bool *connected,
                             bool *dophase_done)
{
  /* This is SMTP and no proxy */
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct SMTP *smtp = data->req.p.smtp;

  DEBUGF(infof(data, "DO phase starts"));

  if(data->set.opt_no_body) {
    /* Requested no body means no transfer */
    smtp->transfer = PPTRANSFER_INFO;
  }

  *dophase_done = FALSE; /* not done yet */

  /* Store the first recipient (or NULL if not specified) */
  smtp->rcpt = data->set.mail_rcpt;

  /* Track of whether we've successfully sent at least one RCPT TO command */
  smtp->rcpt_had_ok = FALSE;

  /* Track of the last error we've received by sending RCPT TO command */
  smtp->rcpt_last_error = 0;

  /* Initial data character is the first character in line: it is implicitly
     preceded by a virtual CRLF. */
  smtp->trailing_crlf = TRUE;
  smtp->eob = 2;

  /* Start the first command in the DO phase */
  if((data->set.upload || data->set.mimepost.kind) && data->set.mail_rcpt)
    /* MAIL transfer */
    result = smtp_perform_mail(data);
  else
    /* SMTP based command (VRFY, EXPN, NOOP, RSET or HELP) */
    result = smtp_perform_command(data);

  if(result)
    return result;

  /* Run the state-machine */
  result = smtp_multi_statemach(data, dophase_done);

  *connected = conn->bits.tcpconnect[FIRSTSOCKET];

  if(*dophase_done)
    DEBUGF(infof(data, "DO phase is complete"));

  return result;
}


// Source: smtp.c
// Lines 1454-1505
