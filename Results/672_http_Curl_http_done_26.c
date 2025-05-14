// Source: curl/lib/http.c
// Lines 1614-1617
CURLcode Curl_http_done(struct Curl_easy *data,
                        CURLcode status, bool premature)
{
  struct connectdata *conn = data->conn;
  struct HTTP *http = data->req.p.http;

  /* Clear multipass flag. If authentication isn't done yet, then it will get
   * a chance to be set back to true when we output the next auth header */
  data->state.authhost.multipass = FALSE;
  data->state.authproxy.multipass = FALSE;

  Curl_unencode_cleanup(data);

  /* set the proper values (possibly modified on POST) */
  conn->seek_func = data->set.seek_func; /* restore */
  conn->seek_client = data->set.seek_client; /* restore */

  if(!http)
    return CURLE_OK;

  Curl_dyn_free(&http->send_buffer);
  Curl_http2_done(data, premature);
  Curl_quic_done(data, premature);
  Curl_mime_cleanpart(&http->form);
  Curl_dyn_reset(&data->state.headerb);
  Curl_hyper_done(data);

  if(status)
    return status;

  if(!premature && /* this check is pointless when DONE is called before the
                      entire operation is complete */
     !conn->bits.retry &&
     !data->set.connect_only &&
     (data->req.bytecount +
      data->req.headerbytecount -
      data->req.deductheadercount) <= 0) {
    /* If this connection isn't simply closed to be retried, AND nothing was
       read from the HTTP server (that counts), this can't be right so we
       return an error here */
    failf(data, "Empty reply from server");
    /* Mark it as closed to avoid the "left intact" message */
    streamclose(conn, "Empty reply from server");
    return CURLE_GOT_NOTHING;
  }

  return CURLE_OK;
}
