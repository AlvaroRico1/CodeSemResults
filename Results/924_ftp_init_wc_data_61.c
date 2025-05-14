// Source: curl/lib/ftp.c
// Lines 3769-3770
static CURLcode init_wc_data(struct Curl_easy *data)
{
  char *last_slash;
  struct FTP *ftp = data->req.p.ftp;
  char *path = ftp->path;
  struct WildcardData *wildcard = &(data->wildcard);
  CURLcode result = CURLE_OK;
  struct ftp_wc *ftpwc = NULL;

  last_slash = strrchr(ftp->path, '/');
  if(last_slash) {
    last_slash++;
    if(last_slash[0] == '\0') {
      wildcard->state = CURLWC_CLEAN;
      result = ftp_parse_url_path(data);
      return result;
    }
    wildcard->pattern = strdup(last_slash);
    if(!wildcard->pattern)
      return CURLE_OUT_OF_MEMORY;
    last_slash[0] = '\0'; /* cut file from path */
  }
  else { /* there is only 'wildcard pattern' or nothing */
    if(path[0]) {
      wildcard->pattern = strdup(path);
      if(!wildcard->pattern)
        return CURLE_OUT_OF_MEMORY;
      path[0] = '\0';
    }
    else { /* only list */
      wildcard->state = CURLWC_CLEAN;
      result = ftp_parse_url_path(data);
      return result;
    }
  }

  /* program continues only if URL is not ending with slash, allocate needed
     resources for wildcard transfer */

  /* allocate ftp protocol specific wildcard data */
  ftpwc = calloc(1, sizeof(struct ftp_wc));
  if(!ftpwc) {
    result = CURLE_OUT_OF_MEMORY;
    goto fail;
  }

  /* INITIALIZE parselist structure */
  ftpwc->parser = Curl_ftp_parselist_data_alloc();
  if(!ftpwc->parser) {
    result = CURLE_OUT_OF_MEMORY;
    goto fail;
  }

  wildcard->protdata = ftpwc; /* put it to the WildcardData tmp pointer */
  wildcard->dtor = wc_data_dtor;

  /* wildcard does not support NOCWD option (assert it?) */
  if(data->set.ftp_filemethod == FTPFILE_NOCWD)
    data->set.ftp_filemethod = FTPFILE_MULTICWD;

  /* try to parse ftp url */
  result = ftp_parse_url_path(data);
  if(result) {
    goto fail;
  }

  wildcard->path = strdup(ftp->path);
  if(!wildcard->path) {
    result = CURLE_OUT_OF_MEMORY;
    goto fail;
  }

  /* backup old write_function */
  ftpwc->backup.write_function = data->set.fwrite_func;
  /* parsing write function */
  data->set.fwrite_func = Curl_ftp_parselist;
  /* backup old file descriptor */
  ftpwc->backup.file_descriptor = data->set.out;
  /* let the writefunc callback know the transfer */
  data->set.out = data;

  infof(data, "Wildcard - Parsing started");
  return CURLE_OK;

  fail:
  if(ftpwc) {
    Curl_ftp_parselist_data_free(&ftpwc->parser);
    free(ftpwc);
  }
  Curl_safefree(wildcard->pattern);
  wildcard->dtor = ZERO_NULL;
  wildcard->protdata = NULL;
  return result;
}
