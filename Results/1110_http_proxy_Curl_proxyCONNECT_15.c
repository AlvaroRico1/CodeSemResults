// Source: curl/lib/http_proxy.c
// Lines 979-985
CURLcode Curl_proxyCONNECT(struct Curl_easy *data,
                           int sockindex,
                           const char *hostname,
                           int remote_port)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  if(!conn->connect_state) {
    result = connect_init(data, FALSE);
    if(result)
      return result;
  }
  result = CONNECT(data, sockindex, hostname, remote_port);

  if(result || Curl_connect_complete(conn))
    connect_done(data);

  return result;
}
