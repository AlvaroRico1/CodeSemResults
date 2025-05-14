// Source: curl/src/tool_cb_hdr.c
// Lines 60-140
size_t tool_header_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  struct per_transfer *per = userdata;
  struct HdrCbData *hdrcbdata = &per->hdrcbdata;
  struct OutStruct *outs = &per->outs;
  struct OutStruct *heads = &per->heads;
  struct OutStruct *etag_save = &per->etag_save;
  const char *str = ptr;
  const size_t cb = size * nmemb;
  const char *end = (char *)ptr + cb;
  long protocol = 0;

  /*
   * Once that libcurl has called back tool_header_cb() the returned value
   * is checked against the amount that was intended to be written, if
   * it does not match then it fails with CURLE_WRITE_ERROR. So at this
   * point returning a value different from sz*nmemb indicates failure.
   */
  size_t failure = (size && nmemb) ? 0 : 1;

  if(!per->config)
    return failure;

#ifdef DEBUGBUILD
  if(size * nmemb > (size_t)CURL_MAX_HTTP_HEADER) {
    warnf(per->config->global, "Header data exceeds single call write "
          "limit!\n");
    return failure;
  }
#endif

  /*
   * Write header data when curl option --dump-header (-D) is given.
   */

  if(per->config->headerfile && heads->stream) {
    size_t rc = fwrite(ptr, size, nmemb, heads->stream);
    if(rc != cb)
      return rc;
    /* flush the stream to send off what we got earlier */
    (void)fflush(heads->stream);
  }

  /*
   * Write etag to file when --etag-save option is given.
   */
  if(per->config->etag_save_file && etag_save->stream) {
    /* match only header that start with etag (case insensitive) */
    if(curl_strnequal(str, "etag:", 5)) {
      const char *etag_h = &str[5];
      const char *eot = end - 1;
      if(*eot == '\n') {
        while(ISSPACE(*etag_h) && (etag_h < eot))
          etag_h++;
        while(ISSPACE(*eot))
          eot--;

        if(eot >= etag_h) {
          size_t etag_length = eot - etag_h + 1;
          fwrite(etag_h, size, etag_length, etag_save->stream);
          /* terminate with newline */
          fputc('\n', etag_save->stream);
          (void)fflush(etag_save->stream);
        }
      }
    }
  }

  /*
   * This callback sets the filename where output shall be written when
   * curl options --remote-name (-O) and --remote-header-name (-J) have
   * been simultaneously given and additionally server returns an HTTP
   * Content-Disposition header specifying a filename property.
   */

  curl_easy_getinfo(per->curl, CURLINFO_PROTOCOL, &protocol);
  if(hdrcbdata->honor_cd_filename &&
     (cb > 20) && checkprefix("Content-disposition:", str) &&
     (protocol & (CURLPROTO_HTTPS|CURLPROTO_HTTP))) {
    const char *p = str + 20;

    /* look for the 'filename=' parameter
       (encoded filenames (*=) are not supported) */
    for(;;) {
      char *filename;
      size_t len;

      while(*p && (p < end) && !ISALPHA(*p))
        p++;
      if(p > end - 9)
        break;

      if(memcmp(p, "filename=", 9)) {
        /* no match, find next parameter */
        while((p < end) && (*p != ';'))
          p++;
        continue;
      }
      p += 9;

      /* this expression below typecasts 'cb' only to avoid
         warning: signed and unsigned type in conditional expression
      */
      len = (ssize_t)cb - (p - str);
      filename = parse_filename(p, len);
      if(filename) {
        if(outs->stream) {
          /* indication of problem, get out! */
          free(filename);
          return failure;
        }

        outs->is_cd_filename = TRUE;
        outs->s_isreg = TRUE;
        outs->fopened = FALSE;
        outs->filename = filename;
        outs->alloc_filename = TRUE;
        hdrcbdata->honor_cd_filename = FALSE; /* done now! */
        if(!tool_create_output_file(outs, per->config))
          return failure;
      }
      break;
    }
