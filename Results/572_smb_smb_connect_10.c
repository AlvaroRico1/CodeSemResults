// Source: curl/lib/smb.c
// Lines 258-259
static CURLcode smb_connect(struct Curl_easy *data, bool *done)
{
  struct connectdata *conn = data->conn;
  struct smb_conn *smbc = &conn->proto.smbc;
  char *slash;

  (void) done;

  /* Check we have a username and password to authenticate with */
  if(!conn->bits.user_passwd)
    return CURLE_LOGIN_DENIED;

  /* Initialize the connection state */
  smbc->state = SMB_CONNECTING;
  smbc->recv_buf = malloc(MAX_MESSAGE_SIZE);
  if(!smbc->recv_buf)
    return CURLE_OUT_OF_MEMORY;

  /* Multiple requests are allowed with this connection */
  connkeep(conn, "SMB default");

  /* Parse the username, domain, and password */
  slash = strchr(conn->user, '/');
  if(!slash)
    slash = strchr(conn->user, '\\');

  if(slash) {
    smbc->user = slash + 1;
    smbc->domain = strdup(conn->user);
    if(!smbc->domain)
      return CURLE_OUT_OF_MEMORY;
    smbc->domain[slash - conn->user] = 0;
  }
  else {
    smbc->user = conn->user;
    smbc->domain = strdup(conn->host.name);
    if(!smbc->domain)
      return CURLE_OUT_OF_MEMORY;
  }

  return CURLE_OK;
}
