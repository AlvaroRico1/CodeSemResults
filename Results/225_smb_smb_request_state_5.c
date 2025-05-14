// Source: curl/lib/smb.c
// Lines 759-761
static CURLcode smb_request_state(struct Curl_easy *data, bool *done)
{
  struct connectdata *conn = data->conn;
  struct smb_request *req = data->req.p.smb;
  struct smb_header *h;
  struct smb_conn *smbc = &conn->proto.smbc;
  enum smb_req_state next_state = SMB_DONE;
  unsigned short len;
  unsigned short off;
  CURLcode result;
  void *msg = NULL;
  const struct smb_nt_create_response *smb_m;

  /* Start the request */
  if(req->state == SMB_REQUESTING) {
    result = smb_send_tree_connect(data);
    if(result) {
      connclose(conn, "SMB: failed to send tree connect message");
      return result;
    }

    request_state(data, SMB_TREE_CONNECT);
  }

  /* Send the previous message and check for a response */
  result = smb_send_and_recv(data, &msg);
  if(result && result != CURLE_AGAIN) {
    connclose(conn, "SMB: failed to communicate");
    return result;
  }

  if(!msg)
    return CURLE_OK;

  h = msg;

  switch(req->state) {
  case SMB_TREE_CONNECT:
    if(h->status) {
      req->result = CURLE_REMOTE_FILE_NOT_FOUND;
      if(h->status == smb_swap32(SMB_ERR_NOACCESS))
        req->result = CURLE_REMOTE_ACCESS_DENIED;
      break;
    }
    req->tid = smb_swap16(h->tid);
    next_state = SMB_OPEN;
    break;

  case SMB_OPEN:
    if(h->status || smbc->got < sizeof(struct smb_nt_create_response)) {
      req->result = CURLE_REMOTE_FILE_NOT_FOUND;
      if(h->status == smb_swap32(SMB_ERR_NOACCESS))
        req->result = CURLE_REMOTE_ACCESS_DENIED;
      next_state = SMB_TREE_DISCONNECT;
      break;
    }
    smb_m = (const struct smb_nt_create_response*) msg;
    req->fid = smb_swap16(smb_m->fid);
    data->req.offset = 0;
    if(data->set.upload) {
      data->req.size = data->state.infilesize;
      Curl_pgrsSetUploadSize(data, data->req.size);
      next_state = SMB_UPLOAD;
    }
    else {
      smb_m = (const struct smb_nt_create_response*) msg;
      data->req.size = smb_swap64(smb_m->end_of_file);
      if(data->req.size < 0) {
        req->result = CURLE_WEIRD_SERVER_REPLY;
        next_state = SMB_CLOSE;
      }
      else {
        Curl_pgrsSetDownloadSize(data, data->req.size);
        if(data->set.get_filetime)
          get_posix_time(&data->info.filetime, smb_m->last_change_time);
        next_state = SMB_DOWNLOAD;
      }
    }
    break;

  case SMB_DOWNLOAD:
    if(h->status || smbc->got < sizeof(struct smb_header) + 14) {
      req->result = CURLE_RECV_ERROR;
      next_state = SMB_CLOSE;
      break;
    }
    len = Curl_read16_le(((const unsigned char *) msg) +
                         sizeof(struct smb_header) + 11);
    off = Curl_read16_le(((const unsigned char *) msg) +
                         sizeof(struct smb_header) + 13);
    if(len > 0) {
      if(off + sizeof(unsigned int) + len > smbc->got) {
        failf(data, "Invalid input packet");
        result = CURLE_RECV_ERROR;
      }
      else
        result = Curl_client_write(data, CLIENTWRITE_BODY,
                                   (char *)msg + off + sizeof(unsigned int),
                                   len);
      if(result) {
        req->result = result;
        next_state = SMB_CLOSE;
        break;
      }
    }
    data->req.bytecount += len;
    data->req.offset += len;
    Curl_pgrsSetDownloadCounter(data, data->req.bytecount);
    next_state = (len < MAX_PAYLOAD_SIZE) ? SMB_CLOSE : SMB_DOWNLOAD;
    break;

  case SMB_UPLOAD:
    if(h->status || smbc->got < sizeof(struct smb_header) + 6) {
      req->result = CURLE_UPLOAD_FAILED;
      next_state = SMB_CLOSE;
      break;
    }
    len = Curl_read16_le(((const unsigned char *) msg) +
                         sizeof(struct smb_header) + 5);
    data->req.bytecount += len;
    data->req.offset += len;
    Curl_pgrsSetUploadCounter(data, data->req.bytecount);
    if(data->req.bytecount >= data->req.size)
      next_state = SMB_CLOSE;
    else
      next_state = SMB_UPLOAD;
    break;

  case SMB_CLOSE:
    /* We don't care if the close failed, proceed to tree disconnect anyway */
    next_state = SMB_TREE_DISCONNECT;
    break;

  case SMB_TREE_DISCONNECT:
    next_state = SMB_DONE;
    break;

  default:
    smb_pop_message(conn);
    return CURLE_OK; /* ignore */
  }

  smb_pop_message(conn);

  switch(next_state) {
  case SMB_OPEN:
    result = smb_send_open(data);
    break;

  case SMB_DOWNLOAD:
    result = smb_send_read(data);
    break;

  case SMB_UPLOAD:
    result = smb_send_write(data);
    break;

  case SMB_CLOSE:
    result = smb_send_close(data);
    break;

  case SMB_TREE_DISCONNECT:
    result = smb_send_tree_disconnect(data);
    break;

  case SMB_DONE:
    result = req->result;
    *done = true;
    break;

  default:
    break;
  }

  if(result) {
    connclose(conn, "SMB: failed to send message");
    return result;
  }

  request_state(data, next_state);

  return CURLE_OK;
}
