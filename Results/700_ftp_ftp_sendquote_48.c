// Source: curl/lib/ftp.c
// Lines 3434-3442
CURLcode ftp_sendquote(struct Curl_easy *data,
                       struct connectdata *conn, struct curl_slist *quote)
{
  struct curl_slist *item;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  struct pingpong *pp = &ftpc->pp;

  item = quote;
  while(item) {
    if(item->data) {
      ssize_t nread;
      char *cmd = item->data;
      bool acceptfail = FALSE;
      CURLcode result;
      int ftpcode = 0;

      /* if a command starts with an asterisk, which a legal FTP command never
         can, the command will be allowed to fail without it causing any
         aborts or cancels etc. It will cause libcurl to act as if the command
         is successful, whatever the server reponds. */

      if(cmd[0] == '*') {
        cmd++;
        acceptfail = TRUE;
      }

      result = Curl_pp_sendf(data, &ftpc->pp, "%s", cmd);
      if(!result) {
        pp->response = Curl_now(); /* timeout relative now */
        result = Curl_GetFTPResponse(data, &nread, &ftpcode);
      }
      if(result)
        return result;

      if(!acceptfail && (ftpcode >= 400)) {
        failf(data, "QUOT string not accepted: %s", cmd);
        return CURLE_QUOTE_ERROR;
      }
    }
