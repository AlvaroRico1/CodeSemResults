static CURLcode ftp_done(struct Curl_easy *data, CURLcode status,
                         bool premature)
{
  struct connectdata *conn = data->conn;
  struct FTP *ftp = data->req.p.ftp;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  struct pingpong *pp = &ftpc->pp;
  ssize_t nread;
  int ftpcode;
  CURLcode result = CURLE_OK;
  char *rawPath = NULL;
  size_t pathLen = 0;

  if(!ftp)
    return CURLE_OK;

  switch(status) {
  case CURLE_BAD_DOWNLOAD_RESUME:
  case CURLE_FTP_WEIRD_PASV_REPLY:
  case CURLE_FTP_PORT_FAILED:
  case CURLE_FTP_ACCEPT_FAILED:
  case CURLE_FTP_ACCEPT_TIMEOUT:
  case CURLE_FTP_COULDNT_SET_TYPE:
  case CURLE_FTP_COULDNT_RETR_FILE:
  case CURLE_PARTIAL_FILE:
  case CURLE_UPLOAD_FAILED:
  case CURLE_REMOTE_ACCESS_DENIED:
  case CURLE_FILESIZE_EXCEEDED:
  case CURLE_REMOTE_FILE_NOT_FOUND:
  case CURLE_WRITE_ERROR:
    /* the connection stays alive fine even though this happened */
    /* fall-through */
  case CURLE_OK: /* doesn't affect the control connection's status */
    if(!premature)
      break;

    /* until we cope better with prematurely ended requests, let them
     * fallback as if in complete failure */
    /* FALLTHROUGH */
  default:       /* by default, an error means the control connection is
                    wedged and should not be used anymore */
    ftpc->ctl_valid = FALSE;
    ftpc->cwdfail = TRUE; /* set this TRUE to prevent us to remember the
                             current path, as this connection is going */
    connclose(conn, "FTP ended with bad error code");
    result = status;      /* use the already set error code */
    break;
  }

  if(data->state.wildcardmatch) {
    if(data->set.chunk_end && ftpc->file) {
      Curl_set_in_callback(data, true);
      data->set.chunk_end(data->wildcard.customptr);
      Curl_set_in_callback(data, false);
    }
    ftpc->known_filesize = -1;
  }

  if(!result)
    /* get the url-decoded "raw" path */
    result = Curl_urldecode(data, ftp->path, 0, &rawPath, &pathLen,
                            REJECT_CTRL);
  if(result) {
    /* We can limp along anyway (and should try to since we may already be in
     * the error path) */
    ftpc->ctl_valid = FALSE; /* mark control connection as bad */
    connclose(conn, "FTP: out of memory!"); /* mark for connection closure */
    free(ftpc->prevpath);
    ftpc->prevpath = NULL; /* no path remembering */
  }
  else { /* remember working directory for connection reuse */
    if((data->set.ftp_filemethod == FTPFILE_NOCWD) && (rawPath[0] == '/'))
      free(rawPath); /* full path => no CWDs happened => keep ftpc->prevpath */
    else {
      free(ftpc->prevpath);

      if(!ftpc->cwdfail) {
        if(data->set.ftp_filemethod == FTPFILE_NOCWD)
          pathLen = 0; /* relative path => working directory is FTP home */
        else
          pathLen -= ftpc->file?strlen(ftpc->file):0; /* file is url-decoded */

        rawPath[pathLen] = '\0';
        ftpc->prevpath = rawPath;
      }
      else {
        free(rawPath);
        ftpc->prevpath = NULL; /* no path */
      }
    }

    if(ftpc->prevpath)
      infof(data, "Remembering we are in dir \"%s\"", ftpc->prevpath);
  }

  /* free the dir tree and file parts */
  freedirs(ftpc);

  /* shut down the socket to inform the server we're done */

#ifdef _WIN32_WCE
  shutdown(conn->sock[SECONDARYSOCKET], 2);  /* SD_BOTH */
#endif

  if(conn->sock[SECONDARYSOCKET] != CURL_SOCKET_BAD) {
    if(!result && ftpc->dont_check && data->req.maxdownload > 0) {
      /* partial download completed */
      result = Curl_pp_sendf(data, pp, "%s", "ABOR");
      if(result) {
        failf(data, "Failure sending ABOR command: %s",
              curl_easy_strerror(result));
        ftpc->ctl_valid = FALSE; /* mark control connection as bad */
        connclose(conn, "ABOR command failed"); /* connection closure */
      }
    }

    if(conn->ssl[SECONDARYSOCKET].use) {
      /* The secondary socket is using SSL so we must close down that part
         first before we close the socket for real */
      Curl_ssl_close(data, conn, SECONDARYSOCKET);

      /* Note that we keep "use" set to TRUE since that (next) connection is
         still requested to use SSL */
    }
    close_secondarysocket(data, conn);
  }

  if(!result && (ftp->transfer == PPTRANSFER_BODY) && ftpc->ctl_valid &&
     pp->pending_resp && !premature) {
    /*
     * Let's see what the server says about the transfer we just performed,
     * but lower the timeout as sometimes this connection has died while the
     * data has been transferred. This happens when doing through NATs etc that
     * abandon old silent connections.
     */
    timediff_t old_time = pp->response_time;

    pp->response_time = 60*1000; /* give it only a minute for now */
    pp->response = Curl_now(); /* timeout relative now */

    result = Curl_GetFTPResponse(data, &nread, &ftpcode);

    pp->response_time = old_time; /* set this back to previous value */

    if(!nread && (CURLE_OPERATION_TIMEDOUT == result)) {
      failf(data, "control connection looks dead");
      ftpc->ctl_valid = FALSE; /* mark control connection as bad */
      connclose(conn, "Timeout or similar in FTP DONE operation"); /* close */
    }

    if(result) {
      Curl_safefree(ftp->pathalloc);
      return result;
    }

    if(ftpc->dont_check && data->req.maxdownload > 0) {
      /* we have just sent ABOR and there is no reliable way to check if it was
       * successful or not; we have to close the connection now */
      infof(data, "partial download completed, closing connection");
      connclose(conn, "Partial download with no ability to check");
      return result;
    }

    if(!ftpc->dont_check) {
      /* 226 Transfer complete, 250 Requested file action okay, completed. */
      switch(ftpcode) {
      case 226:
      case 250:
        break;
      case 552:
        failf(data, "Exceeded storage allocation");
        result = CURLE_REMOTE_DISK_FULL;
        break;
      default:
        failf(data, "server did not report OK, got %d", ftpcode);
        result = CURLE_PARTIAL_FILE;
        break;
      }
    }
  }

  if(result || premature)
    /* the response code from the transfer showed an error already so no
       use checking further */
    ;
  else if(data->set.upload) {
    if((-1 != data->state.infilesize) &&
       (data->state.infilesize != data->req.writebytecount) &&
       !data->set.crlf &&
       (ftp->transfer == PPTRANSFER_BODY)) {
      failf(data, "Uploaded unaligned file size (%" CURL_FORMAT_CURL_OFF_T
            " out of %" CURL_FORMAT_CURL_OFF_T " bytes)",
            data->req.bytecount, data->state.infilesize);
      result = CURLE_PARTIAL_FILE;
    }
  }
  else {
    if((-1 != data->req.size) &&
       (data->req.size != data->req.bytecount) &&
#ifdef CURL_DO_LINEEND_CONV
       /* Most FTP servers don't adjust their file SIZE response for CRLFs, so
        * we'll check to see if the discrepancy can be explained by the number
        * of CRLFs we've changed to LFs.
        */
       ((data->req.size + data->state.crlf_conversions) !=
        data->req.bytecount) &&
#endif /* CURL_DO_LINEEND_CONV */
       (data->req.maxdownload != data->req.bytecount)) {
      failf(data, "Received only partial file: %" CURL_FORMAT_CURL_OFF_T
            " bytes", data->req.bytecount);
      result = CURLE_PARTIAL_FILE;
    }
    else if(!ftpc->dont_check &&
            !data->req.bytecount &&
            (data->req.size>0)) {
      failf(data, "No data was received!");
      result = CURLE_FTP_COULDNT_RETR_FILE;
    }
  }

  /* clear these for next connection */
  ftp->transfer = PPTRANSFER_BODY;
  ftpc->dont_check = FALSE;

  /* Send any post-transfer QUOTE strings? */
  if(!status && !result && !premature && data->set.postquote)
    result = ftp_sendquote(data, conn, data->set.postquote);
  Curl_safefree(ftp->pathalloc);
  return result;
}


// Source: ftp.c
// Lines 3189-3418
