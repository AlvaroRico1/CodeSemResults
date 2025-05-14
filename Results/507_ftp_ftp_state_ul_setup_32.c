// Source: curl/lib/ftp.c
// Lines 1576-1580
static CURLcode ftp_state_ul_setup(struct Curl_easy *data,
                                   bool sizechecked)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct FTP *ftp = data->req.p.ftp;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  bool append = data->set.remote_append;

  if((data->state.resume_from && !sizechecked) ||
     ((data->state.resume_from > 0) && sizechecked)) {
    /* we're about to continue the uploading of a file */
    /* 1. get already existing file's size. We use the SIZE command for this
       which may not exist in the server!  The SIZE command is not in
       RFC959. */

    /* 2. This used to set REST. But since we can do append, we
       don't another ftp command. We just skip the source file
       offset and then we APPEND the rest on the file instead */

    /* 3. pass file-size number of bytes in the source file */
    /* 4. lower the infilesize counter */
    /* => transfer as usual */
    int seekerr = CURL_SEEKFUNC_OK;

    if(data->state.resume_from < 0) {
      /* Got no given size to start from, figure it out */
      result = Curl_pp_sendf(data, &ftpc->pp, "SIZE %s", ftpc->file);
      if(!result)
        state(data, FTP_STOR_SIZE);
      return result;
    }

    /* enable append */
    append = TRUE;

    /* Let's read off the proper amount of bytes from the input. */
    if(conn->seek_func) {
      Curl_set_in_callback(data, true);
      seekerr = conn->seek_func(conn->seek_client, data->state.resume_from,
                                SEEK_SET);
      Curl_set_in_callback(data, false);
    }

    if(seekerr != CURL_SEEKFUNC_OK) {
      curl_off_t passed = 0;
      if(seekerr != CURL_SEEKFUNC_CANTSEEK) {
        failf(data, "Could not seek stream");
        return CURLE_FTP_COULDNT_USE_REST;
      }
      /* seekerr == CURL_SEEKFUNC_CANTSEEK (can't seek to offset) */
      do {
        size_t readthisamountnow =
          (data->state.resume_from - passed > data->set.buffer_size) ?
          (size_t)data->set.buffer_size :
          curlx_sotouz(data->state.resume_from - passed);

        size_t actuallyread =
          data->state.fread_func(data->state.buffer, 1, readthisamountnow,
                                 data->state.in);

        passed += actuallyread;
        if((actuallyread == 0) || (actuallyread > readthisamountnow)) {
          /* this checks for greater-than only to make sure that the
             CURL_READFUNC_ABORT return code still aborts */
          failf(data, "Failed to read data");
          return CURLE_FTP_COULDNT_USE_REST;
        }
      } while(passed < data->state.resume_from);
    }
    /* now, decrease the size of the read */
    if(data->state.infilesize>0) {
      data->state.infilesize -= data->state.resume_from;

      if(data->state.infilesize <= 0) {
        infof(data, "File already completely uploaded");

        /* no data to transfer */
        Curl_setup_transfer(data, -1, -1, FALSE, -1);

        /* Set ->transfer so that we won't get any error in
         * ftp_done() because we didn't transfer anything! */
        ftp->transfer = PPTRANSFER_NONE;

        state(data, FTP_STOP);
        return CURLE_OK;
      }
    }
    /* we've passed, proceed as normal */
  } /* resume_from */

  result = Curl_pp_sendf(data, &ftpc->pp, append?"APPE %s":"STOR %s",
                         ftpc->file);
  if(!result)
    state(data, FTP_STOR);

  return result;
}
