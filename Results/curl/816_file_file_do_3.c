static CURLcode file_do(struct Curl_easy *data, bool *done)
{
  /* This implementation ignores the host name in conformance with
     RFC 1738. Only local files (reachable via the standard file system)
     are supported. This means that files on remotely mounted directories
     (via NFS, Samba, NT sharing) can be accessed through a file:// URL
  */
  CURLcode result = CURLE_OK;
  struct_stat statbuf; /* struct_stat instead of struct stat just to allow the
                          Windows version to have a different struct without
                          having to redefine the simple word 'stat' */
  curl_off_t expected_size = -1;
  bool size_known;
  bool fstated = FALSE;
  char *buf = data->state.buffer;
  curl_off_t bytecount = 0;
  int fd;
  struct FILEPROTO *file;

  *done = TRUE; /* unconditionally */

  Curl_pgrsStartNow(data);

  if(data->set.upload)
    return file_upload(data);

  file = data->req.p.file;

  /* get the fd from the connection phase */
  fd = file->fd;

  /* VMS: This only works reliable for STREAMLF files */
  if(-1 != fstat(fd, &statbuf)) {
    if(!S_ISDIR(statbuf.st_mode))
      expected_size = statbuf.st_size;
    /* and store the modification time */
    data->info.filetime = statbuf.st_mtime;
    fstated = TRUE;
  }

  if(fstated && !data->state.range && data->set.timecondition) {
    if(!Curl_meets_timecondition(data, data->info.filetime)) {
      *done = TRUE;
      return CURLE_OK;
    }
  }

  if(fstated) {
    time_t filetime;
    struct tm buffer;
    const struct tm *tm = &buffer;
    char header[80];
    int headerlen;
    char accept_ranges[24]= { "Accept-ranges: bytes\r\n" };
    if(expected_size >= 0) {
      headerlen = msnprintf(header, sizeof(header),
                "Content-Length: %" CURL_FORMAT_CURL_OFF_T "\r\n",
                expected_size);
      result = Curl_client_write(data, CLIENTWRITE_HEADER, header, headerlen);
      if(result)
        return result;

      result = Curl_client_write(data, CLIENTWRITE_HEADER,
                                 accept_ranges, strlen(accept_ranges));
      if(result != CURLE_OK)
        return result;
    }

    filetime = (time_t)statbuf.st_mtime;
    result = Curl_gmtime(filetime, &buffer);
    if(result)
      return result;

    /* format: "Tue, 15 Nov 1994 12:45:26 GMT" */
    headerlen = msnprintf(header, sizeof(header),
              "Last-Modified: %s, %02d %s %4d %02d:%02d:%02d GMT\r\n%s",
              Curl_wkday[tm->tm_wday?tm->tm_wday-1:6],
              tm->tm_mday,
              Curl_month[tm->tm_mon],
              tm->tm_year + 1900,
              tm->tm_hour,
              tm->tm_min,
              tm->tm_sec,
              data->set.opt_no_body ? "": "\r\n");
    result = Curl_client_write(data, CLIENTWRITE_HEADER, header, headerlen);
    if(result)
      return result;
    /* set the file size to make it available post transfer */
    Curl_pgrsSetDownloadSize(data, expected_size);
    if(data->set.opt_no_body)
      return result;
  }


// Source: file.c
// Lines 362-453
