static CURLcode smb_parse_url_path(struct Curl_easy *data,
                                   struct connectdata *conn)
{
  struct smb_request *req = data->req.p.smb;
  struct smb_conn *smbc = &conn->proto.smbc;
  char *path;
  char *slash;

  /* URL decode the path */
  CURLcode result = Curl_urldecode(data, data->state.up.path, 0, &path, NULL,
                                   REJECT_CTRL);
  if(result)
    return result;

  /* Parse the path for the share */
  smbc->share = strdup((*path == '/' || *path == '\\') ? path + 1 : path);
  free(path);
  if(!smbc->share)
    return CURLE_OUT_OF_MEMORY;

  slash = strchr(smbc->share, '/');
  if(!slash)
    slash = strchr(smbc->share, '\\');

  /* The share must be present */
  if(!slash) {
    Curl_safefree(smbc->share);
    return CURLE_URL_MALFORMAT;
  }

  /* Parse the path for the file path converting any forward slashes into
     backslashes */
  *slash++ = 0;
  req->path = slash;

  for(; *slash; slash++) {
    if(*slash == '/')
      *slash = '\\';
  }
  return CURLE_OK;
}


// Source: smb.c
// Lines 983-1023
