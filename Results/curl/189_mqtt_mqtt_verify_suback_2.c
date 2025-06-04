static CURLcode mqtt_verify_suback(struct Curl_easy *data)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  unsigned char readbuf[MQTT_SUBACK_LEN];
  ssize_t nread;
  struct mqtt_conn *mqtt = &conn->proto.mqtt;

  result = Curl_read(data, sockfd, (char *)readbuf, MQTT_SUBACK_LEN, &nread);
  if(result)
    goto fail;

  Curl_debug(data, CURLINFO_HEADER_IN, (char *)readbuf, (size_t)nread);

  /* fixme */
  if(nread < MQTT_SUBACK_LEN) {
    result = CURLE_WEIRD_SERVER_REPLY;
    goto fail;
  }

  /* verify SUBACK */
  if(readbuf[0] != ((mqtt->packetid >> 8) & 0xff) ||
     readbuf[1] != (mqtt->packetid & 0xff) ||
     readbuf[2] != 0x00)
    result = CURLE_WEIRD_SERVER_REPLY;

fail:
  return result;
}


// Source: mqtt.c
// Lines 441-470
