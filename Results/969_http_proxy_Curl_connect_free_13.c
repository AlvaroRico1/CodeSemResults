// Source: curl/lib/http_proxy.c
// Lines 965-966
void Curl_connect_free(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  struct http_connect_state *s = conn->connect_state;
  if(s) {
    free(s);
    conn->connect_state = NULL;
  }
}
