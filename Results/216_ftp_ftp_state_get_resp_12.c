static CURLcode ftp_state_get_resp(struct Curl_easy *data,
                                   int ftpcode,
                                   ftpstate instate)
{
  CURLcode result = CURLE_OK;
  struct FTP *ftp = data->req.p.ftp;
  struct connectdata *conn = data->conn;

  if((ftpcode == 150) || (ftpcode == 125)) {

    /*
      A;
      150 Opening BINARY mode data connection for /etc/passwd (2241
      bytes).  (ok, the file is being transferred)

      B:
      150 Opening ASCII mode data connection for /bin/ls

      C:
      150 ASCII data connection for /bin/ls (137.167.104.91,37445) (0 bytes).

      D:
      150 Opening ASCII mode data connection for [file] (0.0.0.0,0) (545 bytes)

      E:
      125 Data connection already open; Transfer starting. */

    curl_off_t size = -1; /* default unknown size */


    /*
     * It appears that there are FTP-servers that return size 0 for files when
     * SIZE is used on the file while being in BINARY mode. To work around
     * that (stupid) behavior, we attempt to parse the RETR response even if
     * the SIZE returned size zero.
     *
     * Debugging help from Salvatore Sorrentino on February 26, 2003.
     */

    if((instate != FTP_LIST) &&
       !data->state.prefer_ascii &&
       (ftp->downloadsize < 1)) {
      /*
       * It seems directory listings either don't show the size or very
       * often uses size 0 anyway. ASCII transfers may very well turn out
       * that the transferred amount of data is not the same as this line
       * tells, why using this number in those cases only confuses us.
       *
       * Example D above makes this parsing a little tricky */
      char *bytes;
      char *buf = data->state.buffer;
      bytes = strstr(buf, " bytes");
      if(bytes) {
        long in = (long)(--bytes-buf);
        /* this is a hint there is size information in there! ;-) */
        while(--in) {
          /* scan for the left parenthesis and break there */
          if('(' == *bytes)
            break;
          /* skip only digits */
          if(!ISDIGIT(*bytes)) {
            bytes = NULL;
            break;
          }
          /* one more estep backwards */
          bytes--;
        }
        /* if we have nothing but digits: */
        if(bytes) {
          ++bytes;
          /* get the number! */
          (void)curlx_strtoofft(bytes, NULL, 0, &size);
        }
      }
    }
    else if(ftp->downloadsize > -1)
      size = ftp->downloadsize;

    if(size > data->req.maxdownload && data->req.maxdownload > 0)
      size = data->req.size = data->req.maxdownload;
    else if((instate != FTP_LIST) && (data->state.prefer_ascii))
      size = -1; /* kludge for servers that understate ASCII mode file size */

    infof(data, "Maxdownload = %" CURL_FORMAT_CURL_OFF_T,
          data->req.maxdownload);

    if(instate != FTP_LIST)
      infof(data, "Getting file with size: %" CURL_FORMAT_CURL_OFF_T,
            size);

    /* FTP download: */
    conn->proto.ftpc.state_saved = instate;
    conn->proto.ftpc.retr_size_saved = size;

    if(data->set.ftp_use_port) {
      bool connected;

      result = AllowServerConnect(data, &connected);
      if(result)
        return result;

      if(!connected) {
        struct ftp_conn *ftpc = &conn->proto.ftpc;
        infof(data, "Data conn was not available immediately");
        state(data, FTP_STOP);
        ftpc->wait_data_conn = TRUE;
      }
    }
    else
      return InitiateTransfer(data);
  }
  else {
    if((instate == FTP_LIST) && (ftpcode == 450)) {
      /* simply no matching files in the dir listing */
      ftp->transfer = PPTRANSFER_NONE; /* don't download anything */
      state(data, FTP_STOP); /* this phase is over */
    }
    else {
      failf(data, "RETR response: %03d", ftpcode);
      return instate == FTP_RETR && ftpcode == 550?
        CURLE_REMOTE_FILE_NOT_FOUND:
        CURLE_FTP_COULDNT_RETR_FILE;
    }
  }

  return result;
}


// Source: ftp.c
// Lines 2426-2552
