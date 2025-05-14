// Source: curl/lib/sendf.c
// Lines 289-297
CURLcode Curl_write(struct Curl_easy *data,
                    curl_socket_t sockfd,
                    const void *mem,
                    size_t len,
                    ssize_t *written)
{
  ssize_t bytes_written;
  CURLcode result = CURLE_OK;
  struct connectdata *conn;
  int num;
  DEBUGASSERT(data);
  DEBUGASSERT(data->conn);
  conn = data->conn;
  num = (sockfd == conn->sock[SECONDARYSOCKET]);

#ifdef CURLDEBUG
  {
    /* Allow debug builds to override this logic to force short sends
    */
    char *p = getenv("CURL_SMALLSENDS");
    if(p) {
      size_t altsize = (size_t)strtoul(p, NULL, 10);
      if(altsize)
        len = CURLMIN(len, altsize);
    }
  }
#endif
  bytes_written = conn->send[num](data, num, mem, len, &result);

  *written = bytes_written;
  if(bytes_written >= 0)
    /* we completely ignore the curlcode value when subzero is not returned */
    return CURLE_OK;

  /* handle CURLE_AGAIN or a send failure */
  switch(result) {
  case CURLE_AGAIN:
    *written = 0;
    return CURLE_OK;

  case CURLE_OK:
    /* general send failure */
    return CURLE_SEND_ERROR;

  default:
    /* we got a specific curlcode, forward it */
    return result;
  }
}
