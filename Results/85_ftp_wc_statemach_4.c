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

    case CURLWC_DOWNLOADING: {
      /* filelist has at least one file, lets get first one */
      struct ftp_conn *ftpc = &conn->proto.ftpc;
      struct curl_fileinfo *finfo = wildcard->filelist.head->ptr;
      struct FTP *ftp = data->req.p.ftp;

      char *tmp_path = aprintf("%s%s", wildcard->path, finfo->filename);
      if(!tmp_path)
        return CURLE_OUT_OF_MEMORY;

      /* switch default ftp->path and tmp_path */
      free(ftp->pathalloc);
      ftp->pathalloc = ftp->path = tmp_path;

      infof(data, "Wildcard - START of \"%s\"", finfo->filename);
      if(data->set.chunk_bgn) {
        long userresponse;
        Curl_set_in_callback(data, true);
        userresponse = data->set.chunk_bgn(
          finfo, wildcard->customptr, (int)wildcard->filelist.size);
        Curl_set_in_callback(data, false);
        switch(userresponse) {
        case CURL_CHUNK_BGN_FUNC_SKIP:
          infof(data, "Wildcard - \"%s\" skipped by user",
                finfo->filename);
          wildcard->state = CURLWC_SKIP;
          continue;
        case CURL_CHUNK_BGN_FUNC_FAIL:
          return CURLE_CHUNK_FAILED;
        }
      }

      if(finfo->filetype != CURLFILETYPE_FILE) {
        wildcard->state = CURLWC_SKIP;
        continue;
      }

      if(finfo->flags & CURLFINFOFLAG_KNOWN_SIZE)
        ftpc->known_filesize = finfo->size;

      result = ftp_parse_url_path(data);
      if(result)
        return result;

      /* we don't need the Curl_fileinfo of first file anymore */
      Curl_llist_remove(&wildcard->filelist, wildcard->filelist.head, NULL);

      if(wildcard->filelist.size == 0) { /* remains only one file to down. */
        wildcard->state = CURLWC_CLEAN;
        /* after that will be ftp_do called once again and no transfer
           will be done because of CURLWC_CLEAN state */
        return CURLE_OK;
      }
      return result;
    }

    case CURLWC_SKIP: {
      if(data->set.chunk_end) {
        Curl_set_in_callback(data, true);
        data->set.chunk_end(data->wildcard.customptr);
        Curl_set_in_callback(data, false);
      }
      Curl_llist_remove(&wildcard->filelist, wildcard->filelist.head, NULL);
      wildcard->state = (wildcard->filelist.size == 0) ?
        CURLWC_CLEAN : CURLWC_DOWNLOADING;
      continue;
    }

    case CURLWC_CLEAN: {
      struct ftp_wc *ftpwc = wildcard->protdata;
      result = CURLE_OK;
      if(ftpwc)
        result = Curl_ftp_parselist_geterror(ftpwc->parser);

      wildcard->state = result ? CURLWC_ERROR : CURLWC_DONE;
      return result;
    }

    case CURLWC_DONE:
    case CURLWC_ERROR:
    case CURLWC_CLEAR:
      if(wildcard->dtor)
        wildcard->dtor(wildcard->protdata);
      return result;
    }
  }
  /* UNREACHABLE */
}


// Source: ftp.c
// Lines 3862-3988
