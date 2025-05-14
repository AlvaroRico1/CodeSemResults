// Source: curl/lib/smb.c
// Lines 623-625
static CURLcode smb_send_and_recv(struct Curl_easy *data, void **msg)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  CURLcode result;
  *msg = NULL; /* if it returns early */

  /* Check if there is data in the transfer buffer */
  if(!smbc->send_size && smbc->upload_size) {
    size_t nread = smbc->upload_size > (size_t)data->set.upload_buffer_size ?
      (size_t)data->set.upload_buffer_size : smbc->upload_size;
    data->req.upload_fromhere = data->state.ulbuf;
    result = Curl_fillreadbuffer(data, nread, &nread);
    if(result && result != CURLE_AGAIN)
      return result;
    if(!nread)
      return CURLE_OK;

    smbc->upload_size -= nread;
    smbc->send_size = nread;
    smbc->sent = 0;
  }

  /* Check if there is data to send */
  if(smbc->send_size) {
    result = smb_flush(data);
    if(result)
      return result;
  }

  /* Check if there is still data to be sent */
  if(smbc->send_size || smbc->upload_size)
    return CURLE_AGAIN;

  return smb_recv_message(data, msg);
}
