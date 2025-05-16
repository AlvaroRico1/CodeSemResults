static CURLcode ftp_state_mdtm(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;

  /* Requested time of file or time-depended transfer? */
  if((data->set.get_filetime || data->set.timecondition) && ftpc->file) {

    /* we have requested to get the modified-time of the file, this is a white
       spot as the MDTM is not mentioned in RFC959 */
    result = Curl_pp_sendf(data, &ftpc->pp, "MDTM %s", ftpc->file);

    if(!result)
      state(data, FTP_MDTM);
  }
  else
    result = ftp_state_type(data);

  return result;
}


// Source: ftp.c
// Lines 1552-1572
