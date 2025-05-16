static CURLcode smb_recv_message(struct Curl_easy *data, void **msg)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  char *buf = smbc->recv_buf;
  ssize_t bytes_read;
  size_t nbt_size;
  size_t msg_size;
  size_t len = MAX_MESSAGE_SIZE - smbc->got;
  CURLcode result;

  result = Curl_read(data, FIRSTSOCKET, buf + smbc->got, len, &bytes_read);
  if(result)
    return result;

  if(!bytes_read)
    return CURLE_OK;

  smbc->got += bytes_read;

  /* Check for a 32-bit nbt header */
  if(smbc->got < sizeof(unsigned int))
    return CURLE_OK;

  nbt_size = Curl_read16_be((const unsigned char *)
                            (buf + sizeof(unsigned short))) +
    sizeof(unsigned int);
  if(smbc->got < nbt_size)
    return CURLE_OK;

  msg_size = sizeof(struct smb_header);
  if(nbt_size >= msg_size + 1) {
    /* Add the word count */
    msg_size += 1 + ((unsigned char) buf[msg_size]) * sizeof(unsigned short);
    if(nbt_size >= msg_size + sizeof(unsigned short)) {
      /* Add the byte count */
      msg_size += sizeof(unsigned short) +
        Curl_read16_le((const unsigned char *)&buf[msg_size]);
      if(nbt_size < msg_size)
        return CURLE_READ_ERROR;
    }
  }

  *msg = buf;

  return CURLE_OK;
}


// Source: smb.c
// Lines 299-345
