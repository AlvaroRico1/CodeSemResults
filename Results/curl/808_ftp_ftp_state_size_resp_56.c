static CURLcode ftp_state_size_resp(struct Curl_easy *data,
                                    int ftpcode,
                                    ftpstate instate)
{
  CURLcode result = CURLE_OK;
  curl_off_t filesize = -1;
  char *buf = data->state.buffer;

  /* get the size from the ascii string: */
  if(ftpcode == 213) {
    /* To allow servers to prepend "rubbish" in the response string, we scan
       for all the digits at the end of the response and parse only those as a
       number. */
    char *start = &buf[4];
    char *fdigit = strchr(start, '\r');
    if(fdigit) {
      do
        fdigit--;
      while(ISDIGIT(*fdigit) && (fdigit > start));
      if(!ISDIGIT(*fdigit))
        fdigit++;
    }
    else
      fdigit = start;
    /* ignores parsing errors, which will make the size remain unknown */
    (void)curlx_strtoofft(fdigit, NULL, 0, &filesize);

  }


// Source: ftp.c
// Lines 2287-2314
