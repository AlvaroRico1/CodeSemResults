static CURLcode mqtt_send(struct Curl_easy *data,
                          char *buf, size_t len)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  struct MQTT *mq = data->req.p.mqtt;
  ssize_t n;
  result = Curl_write(data, sockfd, buf, len, &n);
  if(!result)
    Curl_debug(data, CURLINFO_HEADER_OUT, buf, (size_t)n);
  if(len != (size_t)n) {
    size_t nsend = len - n;
    char *sendleftovers = Curl_memdup(&buf[n], nsend);
    if(!sendleftovers)
      return CURLE_OUT_OF_MEMORY;
    mq->sendleftovers = sendleftovers;
    mq->nsend = nsend;
  }


// Source: mqtt.c
// Lines 112-130
