// Source: curl/lib/smb.c
// Lines 354-357
static void smb_format_message(struct Curl_easy *data, struct smb_header *h,
                               unsigned char cmd, size_t len)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  struct smb_request *req = data->req.p.smb;
  unsigned int pid;

  memset(h, 0, sizeof(*h));
  h->nbt_length = htons((unsigned short) (sizeof(*h) - sizeof(unsigned int) +
                                          len));
  memcpy((char *)h->magic, "\xffSMB", 4);
  h->command = cmd;
  h->flags = SMB_FLAGS_CANONICAL_PATHNAMES | SMB_FLAGS_CASELESS_PATHNAMES;
  h->flags2 = smb_swap16(SMB_FLAGS2_IS_LONG_NAME | SMB_FLAGS2_KNOWS_LONG_NAME);
  h->uid = smb_swap16(smbc->uid);
  h->tid = smb_swap16(req->tid);
  pid = getpid();
  h->pid_high = smb_swap16((unsigned short)(pid >> 16));
  h->pid = smb_swap16((unsigned short) pid);
}
