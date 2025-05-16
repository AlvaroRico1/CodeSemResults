CURLcode Curl_write_plain(struct Curl_easy *data,
                          curl_socket_t sockfd,
                          const void *mem,
                          size_t len,
                          ssize_t *written)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  int num;
  DEBUGASSERT(conn);
  num = (sockfd == conn->sock[SECONDARYSOCKET]);

  *written = Curl_send_plain(data, num, mem, len, &result);

  return result;
}


// Source: sendf.c
// Lines 406-421
