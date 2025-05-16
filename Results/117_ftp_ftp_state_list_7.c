static CURLcode ftp_state_list(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct FTP *ftp = data->req.p.ftp;
  struct connectdata *conn = data->conn;

  /* If this output is to be machine-parsed, the NLST command might be better
     to use, since the LIST command output is not specified or standard in any
     way. It has turned out that the NLST list output is not the same on all
     servers either... */

  /*
     if FTPFILE_NOCWD was specified, we should add the path
     as argument for the LIST / NLST / or custom command.
     Whether the server will support this, is uncertain.

     The other ftp_filemethods will CWD into dir/dir/ first and
     then just do LIST (in that case: nothing to do here)
  */
  char *lstArg = NULL;
  char *cmd;

  if((data->set.ftp_filemethod == FTPFILE_NOCWD) && ftp->path) {
    /* url-decode before evaluation: e.g. paths starting/ending with %2f */
    const char *slashPos = NULL;
    char *rawPath = NULL;
    result = Curl_urldecode(data, ftp->path, 0, &rawPath, NULL, REJECT_CTRL);
    if(result)
      return result;

    slashPos = strrchr(rawPath, '/');
    if(slashPos) {
      /* chop off the file part if format is dir/file otherwise remove
         the trailing slash for dir/dir/ except for absolute path / */
      size_t n = slashPos - rawPath;
      if(n == 0)
        ++n;

      lstArg = rawPath;
      lstArg[n] = '\0';
    }
    else
      free(rawPath);
  }


// Source: ftp.c
// Lines 1442-1485
