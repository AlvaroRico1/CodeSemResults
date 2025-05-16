static CURLcode mqtt_read_publish(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  ssize_t nread;
  unsigned char *pkt = (unsigned char *)data->state.buffer;
  size_t remlen;
  struct mqtt_conn *mqtt = &conn->proto.mqtt;
  struct MQTT *mq = data->req.p.mqtt;
  unsigned char packet;

  switch(mqtt->state) {
  MQTT_SUBACK_COMING:
  case MQTT_SUBACK_COMING:
    result = mqtt_verify_suback(data);
    if(result)
      break;

    mqstate(data, MQTT_FIRST, MQTT_PUBWAIT);
    break;

  case MQTT_SUBACK:
  case MQTT_PUBWAIT:
    /* we are expecting PUBLISH or SUBACK */
    packet = mq->firstbyte & 0xf0;
    if(packet == MQTT_MSG_PUBLISH)
      mqstate(data, MQTT_PUB_REMAIN, MQTT_NOSTATE);
    else if(packet == MQTT_MSG_SUBACK) {
      mqstate(data, MQTT_SUBACK_COMING, MQTT_NOSTATE);
      goto MQTT_SUBACK_COMING;
    }
    else if(packet == MQTT_MSG_DISCONNECT) {
      infof(data, "Got DISCONNECT");
      *done = TRUE;
      goto end;
    }
    else {
      result = CURLE_WEIRD_SERVER_REPLY;
      goto end;
    }

    /* -- switched state -- */
    remlen = mq->remaining_length;
    infof(data, "Remaining length: %zd bytes", remlen);
    if(data->set.max_filesize &&
       (curl_off_t)remlen > data->set.max_filesize) {
      failf(data, "Maximum file size exceeded");
      result = CURLE_FILESIZE_EXCEEDED;
      goto end;
    }
    Curl_pgrsSetDownloadSize(data, remlen);
    data->req.bytecount = 0;
    data->req.size = remlen;
    mq->npacket = remlen; /* get this many bytes */
    /* FALLTHROUGH */
  case MQTT_PUB_REMAIN: {
    /* read rest of packet, but no more. Cap to buffer size */
    struct SingleRequest *k = &data->req;
    size_t rest = mq->npacket;
    if(rest > (size_t)data->set.buffer_size)
      rest = (size_t)data->set.buffer_size;
    result = Curl_read(data, sockfd, (char *)pkt, rest, &nread);
    if(result) {
      if(CURLE_AGAIN == result) {
        infof(data, "EEEE AAAAGAIN");
      }
      goto end;
    }
    if(!nread) {
      infof(data, "server disconnected");
      result = CURLE_PARTIAL_FILE;
      goto end;
    }
    Curl_debug(data, CURLINFO_DATA_IN, (char *)pkt, (size_t)nread);

    mq->npacket -= nread;
    k->bytecount += nread;
    Curl_pgrsSetDownloadCounter(data, k->bytecount);

    /* if QoS is set, message contains packet id */

    result = Curl_client_write(data, CLIENTWRITE_BODY, (char *)pkt, nread);
    if(result)
      goto end;

    if(!mq->npacket)
      /* no more PUBLISH payload, back to subscribe wait state */
      mqstate(data, MQTT_FIRST, MQTT_PUBWAIT);
    break;
  }
  default:
    DEBUGASSERT(NULL); /* illegal state */
    result = CURLE_WEIRD_SERVER_REPLY;
    goto end;
  }
  end:
  return result;
}


// Source: mqtt.c
// Lines 581-679
