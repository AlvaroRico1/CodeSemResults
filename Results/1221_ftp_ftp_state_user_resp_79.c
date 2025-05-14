// Source: curl/lib/ftp.c
// Lines 2586-2591
static CURLcode ftp_state_user_resp(struct Curl_easy *data,
                                    int ftpcode,
                                    ftpstate instate)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  (void)instate; /* no use for this yet */

  /* some need password anyway, and others just return 2xx ignored */
  if((ftpcode == 331) && (ftpc->state == FTP_USER)) {
    /* 331 Password required for ...
       (the server requires to send the user's password too) */
    result = Curl_pp_sendf(data, &ftpc->pp, "PASS %s",
                           conn->passwd?conn->passwd:"");
    if(!result)
      state(data, FTP_PASS);
  }
  else if(ftpcode/100 == 2) {
    /* 230 User ... logged in.
       (the user logged in with or without password) */
    result = ftp_state_loggedin(data);
  }
  else if(ftpcode == 332) {
    if(data->set.str[STRING_FTP_ACCOUNT]) {
      result = Curl_pp_sendf(data, &ftpc->pp, "ACCT %s",
                             data->set.str[STRING_FTP_ACCOUNT]);
      if(!result)
        state(data, FTP_ACCT);
    }
    else {
      failf(data, "ACCT requested but none available");
      result = CURLE_LOGIN_DENIED;
    }
  }
  else {
    /* All other response codes, like:

    530 User ... access denied
    (the server denies to log the specified user) */

    if(data->set.str[STRING_FTP_ALTERNATIVE_TO_USER] &&
        !data->state.ftp_trying_alternative) {
      /* Ok, USER failed.  Let's try the supplied command. */
      result =
        Curl_pp_sendf(data, &ftpc->pp, "%s",
                      data->set.str[STRING_FTP_ALTERNATIVE_TO_USER]);
      if(!result) {
        data->state.ftp_trying_alternative = TRUE;
        state(data, FTP_USER);
      }
    }
    else {
      failf(data, "Access denied: %03d", ftpcode);
      result = CURLE_LOGIN_DENIED;
    }
  }
  return result;
}
