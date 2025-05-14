// Source: curl/lib/transfer.c
// Lines 908-911
CURLcode Curl_done_sending(struct Curl_easy *data,
                           struct SingleRequest *k)
{
  struct connectdata *conn = data->conn;
  k->keepon &= ~KEEP_SEND; /* we're done writing */

  /* These functions should be moved into the handler struct! */
  Curl_http2_done_sending(data, conn);
  Curl_quic_done_sending(data);

  if(conn->bits.rewindaftersend) {
    CURLcode result = Curl_readrewind(data);
    if(result)
      return result;
  }
  return CURLE_OK;
}
