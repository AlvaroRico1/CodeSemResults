// Source: curl/lib/http_proxy.c
// Lines 157-160
static CURLcode connect_init(struct Curl_easy *data, bool reinit)
{
  struct http_connect_state *s;
  struct connectdata *conn = data->conn;
  if(!reinit) {
    CURLcode result;
    DEBUGASSERT(!conn->connect_state);
    /* we might need the upload buffer for streaming a partial request */
    result = Curl_get_upload_buffer(data);
    if(result)
      return result;

    s = calloc(1, sizeof(struct http_connect_state));
    if(!s)
      return CURLE_OUT_OF_MEMORY;
    infof(data, "allocate connect buffer!");
    conn->connect_state = s;
    Curl_dyn_init(&s->rcvbuf, DYN_PROXY_CONNECT_HEADERS);

    /* Curl_proxyCONNECT is based on a pointer to a struct HTTP at the
     * member conn->proto.http; we want [protocol] through HTTP and we have
     * to change the member temporarily for connecting to the HTTP
     * proxy. After Curl_proxyCONNECT we have to set back the member to the
     * original pointer
     *
     * This function might be called several times in the multi interface case
     * if the proxy's CONNECT response is not instant.
     */
    s->prot_save = data->req.p.http;
    data->req.p.http = &s->http_proxy;
    connkeep(conn, "HTTP proxy CONNECT");
  }
  else {
    DEBUGASSERT(conn->connect_state);
    s = conn->connect_state;
    Curl_dyn_reset(&s->rcvbuf);
  }
  s->tunnel_state = TUNNEL_INIT;
  s->keepon = KEEPON_CONNECT;
  s->cl = 0;
  s->close_connection = FALSE;
  return CURLE_OK;
}
