static CURLcode smtp_perform_upgrade_tls(struct Curl_easy *data)
{
  /* Start the SSL connection */
  struct connectdata *conn = data->conn;
  struct smtp_conn *smtpc = &conn->proto.smtpc;
  CURLcode result = Curl_ssl_connect_nonblocking(data, conn, FALSE,
                                                 FIRSTSOCKET,
                                                 &smtpc->ssldone);

  if(!result) {
    if(smtpc->state != SMTP_UPGRADETLS)
      state(data, SMTP_UPGRADETLS);

    if(smtpc->ssldone) {
      smtp_to_smtps(conn);
      result = smtp_perform_ehlo(data);
    }
  }

  return result;
}


// Source: smtp.c
// Lines 394-414
