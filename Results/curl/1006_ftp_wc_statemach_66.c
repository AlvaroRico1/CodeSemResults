static CURLcode wc_statemach(struct Curl_easy *data)
{
  struct WildcardData * const wildcard = &(data->wildcard);
  struct connectdata *conn = data->conn;
  CURLcode result = CURLE_OK;

  for(;;) {
    switch(wildcard->state) {
    case CURLWC_INIT:
      result = init_wc_data(data);
      if(wildcard->state == CURLWC_CLEAN)
        /* only listing! */
        return result;
      wildcard->state = result ? CURLWC_ERROR : CURLWC_MATCHING;
      return result;

    case CURLWC_MATCHING: {
      /* In this state is LIST response successfully parsed, so lets restore
         previous WRITEFUNCTION callback and WRITEDATA pointer */
      struct ftp_wc *ftpwc = wildcard->protdata;
      data->set.fwrite_func = ftpwc->backup.write_function;
      data->set.out = ftpwc->backup.file_descriptor;
      ftpwc->backup.write_function = ZERO_NULL;
      ftpwc->backup.file_descriptor = NULL;
      wildcard->state = CURLWC_DOWNLOADING;

      if(Curl_ftp_parselist_geterror(ftpwc->parser)) {
        /* error found in LIST parsing */
        wildcard->state = CURLWC_CLEAN;
        continue;
      }
      if(wildcard->filelist.size == 0) {
        /* no corresponding file */
        wildcard->state = CURLWC_CLEAN;
        return CURLE_REMOTE_FILE_NOT_FOUND;
      }
      continue;
    }


// Source: ftp.c
// Lines 3862-3899
