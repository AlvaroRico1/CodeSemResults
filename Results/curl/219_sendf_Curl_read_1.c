CURLcode Curl_read(struct Curl_easy *data,   /* transfer */
                   curl_socket_t sockfd,     /* read from this socket */
                   char *buf,                /* store read data here */
                   size_t sizerequested,     /* max amount to read */
                   ssize_t *n)               /* amount bytes read */
{
  CURLcode result = CURLE_RECV_ERROR;
  ssize_t nread = 0;
  size_t bytesfromsocket = 0;
  char *buffertofill = NULL;
  struct connectdata *conn = data->conn;

  /* Set 'num' to 0 or 1, depending on which socket that has been sent here.
     If it is the second socket, we set num to 1. Otherwise to 0. This lets
     us use the correct ssl handle. */
  int num = (sockfd == conn->sock[SECONDARYSOCKET]);

  *n = 0; /* reset amount to zero */

  bytesfromsocket = CURLMIN(sizerequested, (size_t)data->set.buffer_size);
  buffertofill = buf;

  nread = conn->recv[num](data, num, buffertofill, bytesfromsocket, &result);
  if(nread < 0)
    return result;

  *n += nread;

  return CURLE_OK;
}


// Source: sendf.c
// Lines 678-707
