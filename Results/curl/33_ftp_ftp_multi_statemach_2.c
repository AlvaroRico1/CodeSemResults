static CURLcode ftp_multi_statemach(struct Curl_easy *data,
                                    bool *done)
{
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  CURLcode result = Curl_pp_statemach(data, &ftpc->pp, FALSE, FALSE);

  /* Check for the state outside of the Curl_socket_check() return code checks
     since at times we are in fact already in this state when this function
     gets called. */
  *done = (ftpc->state == FTP_STOP) ? TRUE : FALSE;

  return result;
}


// Source: ftp.c
// Lines 3106-3119
