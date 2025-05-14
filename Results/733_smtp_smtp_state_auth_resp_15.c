// Source: curl/lib/smtp.c
// Lines 977-982
static CURLcode smtp_state_auth_resp(struct Curl_easy *data,
                                     int smtpcode,
                                     smtpstate instate)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct smtp_conn *smtpc = &conn->proto.smtpc;
  saslprogress progress;

  (void)instate; /* no use for this yet */

  result = Curl_sasl_continue(&smtpc->sasl, data, conn, smtpcode, &progress);
  if(!result)
    switch(progress) {
    case SASL_DONE:
      state(data, SMTP_STOP);  /* Authenticated */
      break;
    case SASL_IDLE:            /* No mechanism left after cancellation */
      failf(data, "Authentication cancelled");
      result = CURLE_LOGIN_DENIED;
      break;
    default:
      break;
    }

  return result;
}
