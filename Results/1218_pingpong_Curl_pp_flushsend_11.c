// Source: curl/lib/pingpong.c
// Lines 473-477
CURLcode Curl_pp_flushsend(struct Curl_easy *data,
                           struct pingpong *pp)
{
  /* we have a piece of a command still left to send */
  struct connectdata *conn = data->conn;
  ssize_t written;
  curl_socket_t sock = conn->sock[FIRSTSOCKET];
  CURLcode result = Curl_write(data, sock, pp->sendthis + pp->sendsize -
                               pp->sendleft, pp->sendleft, &written);
  if(result)
    return result;

  if(written != (ssize_t)pp->sendleft) {
    /* only a fraction was sent */
    pp->sendleft -= written;
  }
  else {
    pp->sendthis = NULL;
    pp->sendleft = pp->sendsize = 0;
    pp->response = Curl_now();
  }
  return CURLE_OK;
}
