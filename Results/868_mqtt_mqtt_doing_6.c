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

  infof(data, "mqtt_doing: state [%d]", (int) mqtt->state);
  switch(mqtt->state) {
  case MQTT_FIRST:
    /* Read the initial byte only */
    result = Curl_read(data, sockfd, (char *)&mq->firstbyte, 1, &nread);
    if(!nread)
      break;
    Curl_debug(data, CURLINFO_HEADER_IN, (char *)&mq->firstbyte, 1);
    /* remember the first byte */
    mq->npacket = 0;
    mqstate(data, MQTT_REMAINING_LENGTH, MQTT_NOSTATE);
    /* FALLTHROUGH */
  case MQTT_REMAINING_LENGTH:
    do {
      result = Curl_read(data, sockfd, (char *)&byte, 1, &nread);
      if(!nread)
        break;
      Curl_debug(data, CURLINFO_HEADER_IN, (char *)&byte, 1);
      pkt[mq->npacket++] = byte;
    } while((byte & 0x80) && (mq->npacket < 4));
    if(nread && (byte & 0x80))
      /* MQTT supports up to 127 * 128^0 + 127 * 128^1 + 127 * 128^2 +
         127 * 128^3 bytes. server tried to send more */
      result = CURLE_WEIRD_SERVER_REPLY;
    if(result)
      break;
    mq->remaining_length = mqtt_decode_len(&pkt[0], mq->npacket, NULL);
    mq->npacket = 0;
    if(mq->remaining_length) {
      mqstate(data, mqtt->nextstate, MQTT_NOSTATE);
      break;
    }
    mqstate(data, MQTT_FIRST, MQTT_FIRST);

    if(mq->firstbyte == MQTT_MSG_DISCONNECT) {
      infof(data, "Got DISCONNECT");
      *done = TRUE;
    }
    break;
  case MQTT_CONNACK:
    result = mqtt_verify_connack(data);
    if(result)
      break;

    if(data->state.httpreq == HTTPREQ_POST) {
      result = mqtt_publish(data);
      if(!result) {
        result = mqtt_disconnect(data);
        *done = TRUE;
      }
      mqtt->nextstate = MQTT_FIRST;
    }
    else {
      result = mqtt_subscribe(data);
      if(!result) {
        mqstate(data, MQTT_FIRST, MQTT_SUBACK);
      }
    }
    break;

  case MQTT_SUBACK:
  case MQTT_PUBWAIT:
  case MQTT_PUB_REMAIN:
    result = mqtt_read_publish(data, done);
    break;

  default:
    failf(data, "State not handled yet");
    *done = TRUE;
    break;
  }

  if(result == CURLE_AGAIN)
    result = CURLE_OK;
  return result;
}


// Source: mqtt.c
// Lines 695-792
