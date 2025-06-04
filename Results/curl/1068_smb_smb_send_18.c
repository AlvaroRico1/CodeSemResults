static CURLcode smb_send(struct Curl_easy *data, ssize_t len,
                         size_t upload_size)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  ssize_t bytes_written;
  CURLcode result;

  result = Curl_write(data, FIRSTSOCKET, data->state.ulbuf,
                      len, &bytes_written);
  if(result)
    return result;

  if(bytes_written != len) {
    smbc->send_size = len;
    smbc->sent = bytes_written;
  }

  smbc->upload_size = upload_size;

  return CURLE_OK;
}


// Source: smb.c
// Lines 376-397
