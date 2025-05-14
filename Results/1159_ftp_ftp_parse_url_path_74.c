// Source: curl/lib/ftp.c
// Lines 4119-4121
CURLcode ftp_parse_url_path(struct Curl_easy *data)
{
  /* the ftp struct is already inited in ftp_connect() */
  struct FTP *ftp = data->req.p.ftp;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  const char *slashPos = NULL;
  const char *fileName = NULL;
  CURLcode result = CURLE_OK;
  char *rawPath = NULL; /* url-decoded "raw" path */
  size_t pathLen = 0;

  ftpc->ctl_valid = FALSE;
  ftpc->cwdfail = FALSE;

  /* url-decode ftp path before further evaluation */
  result = Curl_urldecode(data, ftp->path, 0, &rawPath, &pathLen, REJECT_CTRL);
  if(result)
    return result;

  switch(data->set.ftp_filemethod) {
    case FTPFILE_NOCWD: /* fastest, but less standard-compliant */

      if((pathLen > 0) && (rawPath[pathLen - 1] != '/'))
          fileName = rawPath;  /* this is a full file path */
      /*
        else: ftpc->file is not used anywhere other than for operations on
              a file. In other words, never for directory operations.
              So we can safely leave filename as NULL here and use it as a
              argument in dir/file decisions.
      */
      break;

    case FTPFILE_SINGLECWD:
      slashPos = strrchr(rawPath, '/');
      if(slashPos) {
        /* get path before last slash, except for / */
        size_t dirlen = slashPos - rawPath;
        if(dirlen == 0)
            dirlen++;

        ftpc->dirs = calloc(1, sizeof(ftpc->dirs[0]));
        if(!ftpc->dirs) {
          free(rawPath);
          return CURLE_OUT_OF_MEMORY;
        }

        ftpc->dirs[0] = calloc(1, dirlen + 1);
        if(!ftpc->dirs[0]) {
          free(rawPath);
          return CURLE_OUT_OF_MEMORY;
        }

        strncpy(ftpc->dirs[0], rawPath, dirlen);
        ftpc->dirdepth = 1; /* we consider it to be a single dir */
        fileName = slashPos + 1; /* rest is file name */
      }
      else
        fileName = rawPath; /* file name only (or empty) */
      break;

    default: /* allow pretty much anything */
    case FTPFILE_MULTICWD: {
      /* current position: begin of next path component */
      const char *curPos = rawPath;

      int dirAlloc = 0; /* number of entries allocated for the 'dirs' array */
      const char *str = rawPath;
      for(; *str != 0; ++str)
        if (*str == '/')
          ++dirAlloc;

      if(dirAlloc > 0) {
        ftpc->dirs = calloc(dirAlloc, sizeof(ftpc->dirs[0]));
        if(!ftpc->dirs) {
          free(rawPath);
          return CURLE_OUT_OF_MEMORY;
        }

        /* parse the URL path into separate path components */
        while((slashPos = strchr(curPos, '/')) != NULL) {
          size_t compLen = slashPos - curPos;

          /* path starts with a slash: add that as a directory */
          if((compLen == 0) && (ftpc->dirdepth == 0))
            ++compLen;

          /* we skip empty path components, like "x//y" since the FTP command
             CWD requires a parameter and a non-existent parameter a) doesn't
             work on many servers and b) has no effect on the others. */
          if(compLen > 0) {
            char *comp = calloc(1, compLen + 1);
            if(!comp) {
              free(rawPath);
              return CURLE_OUT_OF_MEMORY;
            }
            strncpy(comp, curPos, compLen);
            ftpc->dirs[ftpc->dirdepth++] = comp;
          }
          curPos = slashPos + 1;
        }
      }
      DEBUGASSERT(ftpc->dirdepth <= dirAlloc);
      fileName = curPos; /* the rest is the file name (or empty) */
    }
    break;
  } /* switch */

  if(fileName && *fileName)
    ftpc->file = strdup(fileName);
  else
    ftpc->file = NULL; /* instead of point to a zero byte,
                            we make it a NULL pointer */

  if(data->set.upload && !ftpc->file && (ftp->transfer == PPTRANSFER_BODY)) {
    /* We need a file name when uploading. Return error! */
    failf(data, "Uploading to a URL without a file name!");
    free(rawPath);
    return CURLE_URL_MALFORMAT;
  }

  ftpc->cwddone = FALSE; /* default to not done */

  if((data->set.ftp_filemethod == FTPFILE_NOCWD) && (rawPath[0] == '/'))
    ftpc->cwddone = TRUE; /* skip CWD for absolute paths */
  else { /* newly created FTP connections are already in entry path */
    const char *oldPath = conn->bits.reuse ? ftpc->prevpath : "";
    if(oldPath) {
      size_t n = pathLen;
      if(data->set.ftp_filemethod == FTPFILE_NOCWD)
        n = 0; /* CWD to entry for relative paths */
      else
        n -= ftpc->file?strlen(ftpc->file):0;

      if((strlen(oldPath) == n) && !strncmp(rawPath, oldPath, n)) {
        infof(data, "Request has same path as previous transfer");
        ftpc->cwddone = TRUE;
      }
    }
  }

  free(rawPath);
  return CURLE_OK;
}
