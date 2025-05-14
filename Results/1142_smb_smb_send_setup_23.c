// Source: curl/lib/smb.c
// Lines 450-456
static CURLcode smb_send_setup(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  struct smb_setup msg;
  char *p = msg.bytes;
  unsigned char lm_hash[21];
  unsigned char lm[24];
  unsigned char nt_hash[21];
  unsigned char nt[24];

  size_t byte_count = sizeof(lm) + sizeof(nt);
  byte_count += strlen(smbc->user) + strlen(smbc->domain);
  byte_count += strlen(OS) + strlen(CLIENTNAME) + 4; /* 4 null chars */
  if(byte_count > sizeof(msg.bytes))
    return CURLE_FILESIZE_EXCEEDED;

  Curl_ntlm_core_mk_lm_hash(data, conn->passwd, lm_hash);
  Curl_ntlm_core_lm_resp(lm_hash, smbc->challenge, lm);
#ifdef USE_NTRESPONSES
  Curl_ntlm_core_mk_nt_hash(data, conn->passwd, nt_hash);
  Curl_ntlm_core_lm_resp(nt_hash, smbc->challenge, nt);
#else
  memset(nt, 0, sizeof(nt));
#endif

  memset(&msg, 0, sizeof(msg));
  msg.word_count = SMB_WC_SETUP_ANDX;
  msg.andx.command = SMB_COM_NO_ANDX_COMMAND;
  msg.max_buffer_size = smb_swap16(MAX_MESSAGE_SIZE);
  msg.max_mpx_count = smb_swap16(1);
  msg.vc_number = smb_swap16(1);
  msg.session_key = smb_swap32(smbc->session_key);
  msg.capabilities = smb_swap32(SMB_CAP_LARGE_FILES);
  msg.lengths[0] = smb_swap16(sizeof(lm));
  msg.lengths[1] = smb_swap16(sizeof(nt));
  memcpy(p, lm, sizeof(lm));
  p += sizeof(lm);
  memcpy(p, nt, sizeof(nt));
  p += sizeof(nt);
  MSGCATNULL(smbc->user);
  MSGCATNULL(smbc->domain);
  MSGCATNULL(OS);
  MSGCATNULL(CLIENTNAME);
  byte_count = p - msg.bytes;
  msg.byte_count = smb_swap16((unsigned short)byte_count);

  return smb_send_message(data, SMB_COM_SETUP_ANDX, &msg,
                          sizeof(msg) - sizeof(msg.bytes) + byte_count);
}
