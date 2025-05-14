// Source: curl/lib/pingpong.c
// Lines 458-461
int Curl_pp_getsock(struct Curl_easy *data,
                    struct pingpong *pp, curl_socket_t *socks)
{
  struct connectdata *conn = data->conn;
  socks[0] = conn->sock[FIRSTSOCKET];

  if(pp->sendleft) {
    /* write mode */
    return GETSOCK_WRITESOCK(0);
  }

  /* read mode */
  return GETSOCK_READSOCK(0);
}
