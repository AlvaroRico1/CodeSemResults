// Source: curl/lib/pop3.c
// Lines 568-571
static CURLcode pop3_perform_command(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct POP3 *pop3 = data->req.p.pop3;
  const char *command = NULL;

  /* Calculate the default command */
  if(pop3->id[0] == '\0' || data->set.list_only) {
    command = "LIST";

    if(pop3->id[0] != '\0')
      /* Message specific LIST so skip the BODY transfer */
      pop3->transfer = PPTRANSFER_INFO;
  }
  else
    command = "RETR";

  /* Send the command */
  if(pop3->id[0] != '\0')
    result = Curl_pp_sendf(data, &conn->proto.pop3c.pp, "%s %s",
                           (pop3->custom && pop3->custom[0] != '\0' ?
                            pop3->custom : command), pop3->id);
  else
    result = Curl_pp_sendf(data, &conn->proto.pop3c.pp, "%s",
                           (pop3->custom && pop3->custom[0] != '\0' ?
                            pop3->custom : command));

  if(!result)
    state(data, POP3_COMMAND);

  return result;
}
