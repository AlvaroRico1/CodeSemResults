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


// Source: transfer.c
// Lines 415-436
