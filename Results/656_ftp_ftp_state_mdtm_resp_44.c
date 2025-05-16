static CURLcode ftp_state_mdtm_resp(struct Curl_easy *data,
                                    int ftpcode)
{
  CURLcode result = CURLE_OK;
  struct FTP *ftp = data->req.p.ftp;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;

  switch(ftpcode) {
  case 213:
    {
      /* we got a time. Format should be: "YYYYMMDDHHMMSS[.sss]" where the
         last .sss part is optional and means fractions of a second */
      int year, month, day, hour, minute, second;
      if(6 == sscanf(&data->state.buffer[4], "%04d%02d%02d%02d%02d%02d",
                     &year, &month, &day, &hour, &minute, &second)) {
        /* we have a time, reformat it */
        char timebuf[24];
        msnprintf(timebuf, sizeof(timebuf),
                  "%04d%02d%02d %02d:%02d:%02d GMT",
                  year, month, day, hour, minute, second);
        /* now, convert this into a time() value: */
        data->info.filetime = Curl_getdate_capped(timebuf);
      }

#ifdef CURL_FTP_HTTPSTYLE_HEAD
      /* If we asked for a time of the file and we actually got one as well,
         we "emulate" a HTTP-style header in our output. */

      if(data->set.opt_no_body &&
         ftpc->file &&
         data->set.get_filetime &&
         (data->info.filetime >= 0) ) {
        char headerbuf[128];
        int headerbuflen;
        time_t filetime = data->info.filetime;
        struct tm buffer;
        const struct tm *tm = &buffer;

        result = Curl_gmtime(filetime, &buffer);
        if(result)
          return result;

        /* format: "Tue, 15 Nov 1994 12:45:26" */
        headerbuflen = msnprintf(headerbuf, sizeof(headerbuf),
                  "Last-Modified: %s, %02d %s %4d %02d:%02d:%02d GMT\r\n",
                  Curl_wkday[tm->tm_wday?tm->tm_wday-1:6],
                  tm->tm_mday,
                  Curl_month[tm->tm_mon],
                  tm->tm_year + 1900,
                  tm->tm_hour,
                  tm->tm_min,
                  tm->tm_sec);
        result = Curl_client_write(data, CLIENTWRITE_BOTH, headerbuf,
                                   headerbuflen);
        if(result)
          return result;
      } /* end of a ridiculous amount of conditionals */
#endif
    }
    break;
  default:
    infof(data, "unsupported MDTM reply format");
    break;
  case 550: /* "No such file or directory" */
    failf(data, "Given file does not exist");
    result = CURLE_REMOTE_FILE_NOT_FOUND;
    break;
  }

  if(data->set.timecondition) {
    if((data->info.filetime > 0) && (data->set.timevalue > 0)) {
      switch(data->set.timecondition) {
      case CURL_TIMECOND_IFMODSINCE:
      default:
        if(data->info.filetime <= data->set.timevalue) {
          infof(data, "The requested document is not new enough");
          ftp->transfer = PPTRANSFER_NONE; /* mark to not transfer data */
          data->info.timecond = TRUE;
          state(data, FTP_STOP);
          return CURLE_OK;
        }
        break;
      case CURL_TIMECOND_IFUNMODSINCE:
        if(data->info.filetime > data->set.timevalue) {
          infof(data, "The requested document is not old enough");
          ftp->transfer = PPTRANSFER_NONE; /* mark to not transfer data */
          data->info.timecond = TRUE;
          state(data, FTP_STOP);
          return CURLE_OK;
        }
        break;
      } /* switch */
    }
    else {
      infof(data, "Skipping time comparison");
    }
  }

  if(!result)
    result = ftp_state_type(data);

  return result;
}


// Source: ftp.c
// Lines 2069-2172
