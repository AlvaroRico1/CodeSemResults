static CURLcode mqtt_subscribe(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  char *topic = NULL;
  size_t topiclen;
  unsigned char *packet = NULL;
  size_t packetlen;
  char encodedsize[4];
  size_t n;
  struct connectdata *conn = data->conn;

  result = mqtt_get_topic(data, &topic, &topiclen);
  if(result)
    goto fail;

  conn->proto.mqtt.packetid++;

  packetlen = topiclen + 5; /* packetid + topic (has a two byte length field)
                               + 2 bytes topic length + QoS byte */
  n = mqtt_encode_len((char *)encodedsize, packetlen);
  packetlen += n + 1; /* add one for the control packet type byte */

  packet = malloc(packetlen);
  if(!packet) {
    result = CURLE_OUT_OF_MEMORY;
    goto fail;
  }

  packet[0] = MQTT_MSG_SUBSCRIBE;
  memcpy(&packet[1], encodedsize, n);
  packet[1 + n] = (conn->proto.mqtt.packetid >> 8) & 0xff;
  packet[2 + n] = conn->proto.mqtt.packetid & 0xff;
  packet[3 + n] = (topiclen >> 8) & 0xff;
  packet[4 + n ] = topiclen & 0xff;
  memcpy(&packet[5 + n], topic, topiclen);
  packet[5 + n + topiclen] = 0; /* QoS zero */

  result = mqtt_send(data, (char *)packet, packetlen);

fail:
  free(topic);
  free(packet);
  return result;
}


// Source: mqtt.c
// Lines 393-436
