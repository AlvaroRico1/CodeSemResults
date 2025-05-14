// Source: curl/lib/ftp.c
// Lines 1356-1360
static CURLcode ftp_state_prepare_transfer(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct FTP *ftp = data->req.p.ftp;
  struct connectdata *conn = data->conn;

  if(ftp->transfer != PPTRANSFER_BODY) {
    /* doesn't transfer any data */

    /* still possibly do PRE QUOTE jobs */
    state(data, FTP_RETR_PREQUOTE);
    result = ftp_state_quote(data, TRUE, FTP_RETR_PREQUOTE);
  }
  else if(data->set.ftp_use_port) {
    /* We have chosen to use the PORT (or similar) command */
    result = ftp_state_use_port(data, EPRT);
  }
  else {
    /* We have chosen (this is default) to use the PASV (or similar) command */
    if(data->set.ftp_use_pret) {
      /* The user has requested that we send a PRET command
         to prepare the server for the upcoming PASV */
      struct ftp_conn *ftpc = &conn->proto.ftpc;
      if(!conn->proto.ftpc.file)
        result = Curl_pp_sendf(data, &ftpc->pp, "PRET %s",
                               data->set.str[STRING_CUSTOMREQUEST]?
                               data->set.str[STRING_CUSTOMREQUEST]:
                               (data->state.list_only?"NLST":"LIST"));
      else if(data->set.upload)
        result = Curl_pp_sendf(data, &ftpc->pp, "PRET STOR %s",
                               conn->proto.ftpc.file);
      else
        result = Curl_pp_sendf(data, &ftpc->pp, "PRET RETR %s",
                               conn->proto.ftpc.file);
      if(!result)
        state(data, FTP_PRET);
    }
    else
      result = ftp_state_use_pasv(data, conn);
  }
  return result;
}
