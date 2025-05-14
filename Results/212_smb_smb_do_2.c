// Source: curl/lib/smb.c
// Lines 971-973
static CURLcode smb_do(struct Curl_easy *data, bool *done)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;

  *done = FALSE;
  if(smbc->share) {
    return CURLE_OK;
  }
  return CURLE_URL_MALFORMAT;
}
