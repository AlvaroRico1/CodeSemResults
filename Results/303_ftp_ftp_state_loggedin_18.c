static CURLcode ftp_state_loggedin(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  if(conn->bits.ftp_use_control_ssl) {
    /* PBSZ = PROTECTION BUFFER SIZE.

    The 'draft-murray-auth-ftp-ssl' (draft 12, page 7) says:

    Specifically, the PROT command MUST be preceded by a PBSZ
    command and a PBSZ command MUST be preceded by a successful
    security data exchange (the TLS negotiation in this case)

    ... (and on page 8):

    Thus the PBSZ command must still be issued, but must have a
    parameter of '0' to indicate that no buffering is taking place
    and the data connection should not be encapsulated.
    */
    result = Curl_pp_sendf(data, &conn->proto.ftpc.pp, "PBSZ %d", 0);
    if(!result)
      state(data, FTP_PBSZ);
  }
  else {
    result = ftp_state_pwd(data, conn);
  }
  return result;
}


// Source: ftp.c
// Lines 2555-2583
