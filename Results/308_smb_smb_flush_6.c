// Source: curl/lib/smb.c
// Lines 399-401
static CURLcode smb_flush(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  ssize_t bytes_written;
  ssize_t len = smbc->send_size - smbc->sent;
  CURLcode result;

  if(!smbc->send_size)
    return CURLE_OK;

  result = Curl_write(data, FIRSTSOCKET,
                      data->state.ulbuf + smbc->sent,
                      len, &bytes_written);
  if(result)
    return result;

  if(bytes_written != len)
    smbc->sent += bytes_written;
  else
    smbc->send_size = 0;

  return CURLE_OK;
}
