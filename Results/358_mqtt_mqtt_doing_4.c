// Source: curl/lib/mqtt.c
// Lines 700-710
static CURLcode mqtt_doing(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct mqtt_conn *mqtt = &conn->proto.mqtt;
  struct MQTT *mq = data->req.p.mqtt;
  ssize_t nread;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  unsigned char *pkt = (unsigned char *)data->state.buffer;
  unsigned char byte;

  *done = FALSE;

  if(mq->nsend) {
    /* send the remainder of an outgoing packet */
    char *ptr = mq->sendleftovers;
    result = mqtt_send(data, mq->sendleftovers, mq->nsend);
    free(ptr);
    if(result)
      return result;
  }
