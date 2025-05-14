// Source: curl/lib/hostip.c
// Lines 1240-1243
CURLcode Curl_once_resolved(struct Curl_easy *data, bool *protocol_done)
{
  CURLcode result;
  struct connectdata *conn = data->conn;

#ifdef USE_CURL_ASYNC
  if(data->state.async.dns) {
    conn->dns_entry = data->state.async.dns;
    data->state.async.dns = NULL;
  }
#endif

  result = Curl_setup_conn(data, protocol_done);

  if(result) {
    Curl_detach_connnection(data);
    Curl_conncache_remove_conn(data, conn, TRUE);
    Curl_disconnect(data, conn, TRUE);
  }
  return result;
}
