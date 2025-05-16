static CURLcode pop3_state_user_resp(struct Curl_easy *data, int pop3code,
                                     pop3state instate)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  (void)instate; /* no use for this yet */

  if(pop3code != '+') {
    failf(data, "Access denied. %c", pop3code);
    result = CURLE_LOGIN_DENIED;
  }
  else
    /* Send the PASS command */
    result = Curl_pp_sendf(data, &conn->proto.pop3c.pp, "PASS %s",
                           conn->passwd ? conn->passwd : "");
  if(!result)
    state(data, POP3_PASS);

  return result;
}


// Source: pop3.c
// Lines 853-872
