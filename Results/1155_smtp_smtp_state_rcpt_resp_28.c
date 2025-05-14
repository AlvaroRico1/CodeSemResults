// Source: curl/lib/smtp.c
// Lines 1074-1074
static CURLcode smtp_state_rcpt_resp(struct Curl_easy *data,
                                     struct connectdata *conn, int smtpcode,
                                     smtpstate instate)
{
  CURLcode result = CURLE_OK;
  struct SMTP *smtp = data->req.p.smtp;
  bool is_smtp_err = FALSE;
  bool is_smtp_blocking_err = FALSE;

  (void)instate; /* no use for this yet */

  is_smtp_err = (smtpcode/100 != 2) ? TRUE : FALSE;

  /* If there's multiple RCPT TO to be issued, it's possible to ignore errors
     and proceed with only the valid addresses. */
  is_smtp_blocking_err =
    (is_smtp_err && !data->set.mail_rcpt_allowfails) ? TRUE : FALSE;

  if(is_smtp_err) {
    /* Remembering the last failure which we can report if all "RCPT TO" have
       failed and we cannot proceed. */
    smtp->rcpt_last_error = smtpcode;

    if(is_smtp_blocking_err) {
      failf(data, "RCPT failed: %d", smtpcode);
      result = CURLE_SEND_ERROR;
    }
  }
  else {
    /* Some RCPT TO commands have succeeded. */
    smtp->rcpt_had_ok = TRUE;
  }

  if(!is_smtp_blocking_err) {
    smtp->rcpt = smtp->rcpt->next;

    if(smtp->rcpt)
      /* Send the next RCPT TO command */
      result = smtp_perform_rcpt_to(data);
    else {
      /* We weren't able to issue a successful RCPT TO command while going
         over recipients (potentially multiple). Sending back last error. */
      if(!smtp->rcpt_had_ok) {
        failf(data, "RCPT failed: %d (last error)", smtp->rcpt_last_error);
        result = CURLE_SEND_ERROR;
      }
      else {
        /* Send the DATA command */
        result = Curl_pp_sendf(data, &conn->proto.smtpc.pp, "%s", "DATA");

        if(!result)
          state(data, SMTP_DATA);
      }
    }
  }

  return result;
}
