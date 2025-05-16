static CURLcode mqtt_verify_connack(struct Curl_easy *data)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
  unsigned char readbuf[MQTT_CONNACK_LEN];
  ssize_t nread;

  result = Curl_read(data, sockfd, (char *)readbuf, MQTT_CONNACK_LEN, &nread);
  if(result)
    goto fail;

  Curl_debug(data, CURLINFO_HEADER_IN, (char *)readbuf, (size_t)nread);

  /* fixme */
  if(nread < MQTT_CONNACK_LEN) {
    result = CURLE_WEIRD_SERVER_REPLY;
    goto fail;
  }

  /* verify CONNACK */
  if(readbuf[0] != 0x00 || readbuf[1] != 0x00) {
    failf(data, "Expected %02x%02x but got %02x%02x",
          0x00, 0x00, readbuf[0], readbuf[1]);
    result = CURLE_WEIRD_SERVER_REPLY;
  }

fail:
  return result;
}


// Source: mqtt.c
// Lines 351-380
