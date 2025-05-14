// Source: curl/lib/smtp.c
// Lines 464-467
static CURLcode smtp_perform_authentication(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct smtp_conn *smtpc = &conn->proto.smtpc;
  saslprogress progress;

  /* Check we have enough data to authenticate with, and the
     server supports authentiation, and end the connect phase if not */
  if(!smtpc->auth_supported ||
     !Curl_sasl_can_authenticate(&smtpc->sasl, conn)) {
    state(data, SMTP_STOP);
    return result;
  }

  /* Calculate the SASL login details */
  result = Curl_sasl_start(&smtpc->sasl, data, conn, FALSE, &progress);

  if(!result) {
    if(progress == SASL_INPROGRESS)
      state(data, SMTP_AUTH);
    else {
      /* Other mechanisms not supported */
      infof(data, "No known authentication mechanisms supported!");
      result = CURLE_LOGIN_DENIED;
    }
  }

  return result;
}
