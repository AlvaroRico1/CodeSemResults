// Source: curl/lib/transfer.c
// Lines 415-417
CURLcode Curl_readrewind(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  curl_mimepart *mimepart = &data->set.mimepost;

  conn->bits.rewindaftersend = FALSE; /* we rewind now */

  /* explicitly switch off sending data on this connection now since we are
     about to restart a new transfer and thus we want to avoid inadvertently
     sending more data on the existing connection until the next transfer
     starts */
  data->req.keepon &= ~KEEP_SEND;

  /* We have sent away data. If not using CURLOPT_POSTFIELDS or
     CURLOPT_HTTPPOST, call app to rewind
  */
  if(conn->handler->protocol & PROTO_FAMILY_HTTP) {
    struct HTTP *http = data->req.p.http;

    if(http->sendit)
      mimepart = http->sendit;
  }
  if(data->set.postfields)
    ; /* do nothing */
  else if(data->state.httpreq == HTTPREQ_POST_MIME ||
          data->state.httpreq == HTTPREQ_POST_FORM) {
    CURLcode result = Curl_mime_rewind(mimepart);
    if(result) {
      failf(data, "Cannot rewind mime/post data");
      return result;
    }
  }
  else {
    if(data->set.seek_func) {
      int err;

      Curl_set_in_callback(data, true);
      err = (data->set.seek_func)(data->set.seek_client, 0, SEEK_SET);
      Curl_set_in_callback(data, false);
      if(err) {
        failf(data, "seek callback returned error %d", (int)err);
        return CURLE_SEND_FAIL_REWIND;
      }
    }
    else if(data->set.ioctl_func) {
      curlioerr err;

      Curl_set_in_callback(data, true);
      err = (data->set.ioctl_func)(data, CURLIOCMD_RESTARTREAD,
                                   data->set.ioctl_client);
      Curl_set_in_callback(data, false);
      infof(data, "the ioctl callback returned %d", (int)err);

      if(err) {
        failf(data, "ioctl callback returned error %d", (int)err);
        return CURLE_SEND_FAIL_REWIND;
      }
    }
    else {
      /* If no CURLOPT_READFUNCTION is used, we know that we operate on a
         given FILE * stream and we can actually attempt to rewind that
         ourselves with fseek() */
      if(data->state.fread_func == (curl_read_callback)fread) {
        if(-1 != fseek(data->state.in, 0, SEEK_SET))
          /* successful rewind */
          return CURLE_OK;
      }

      /* no callback set or failure above, makes us fail at once */
      failf(data, "necessary data rewind wasn't possible");
      return CURLE_SEND_FAIL_REWIND;
    }
  }
  return CURLE_OK;
}
