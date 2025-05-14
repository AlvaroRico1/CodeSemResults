// Source: curl/lib/pop3.c
// Lines 1166-1171
static CURLcode pop3_perform(struct Curl_easy *data, bool *connected,
                             bool *dophase_done)
{
  /* This is POP3 and no proxy */
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct POP3 *pop3 = data->req.p.pop3;

  DEBUGF(infof(data, "DO phase starts"));

  if(data->set.opt_no_body) {
    /* Requested no body means no transfer */
    pop3->transfer = PPTRANSFER_INFO;
  }

  *dophase_done = FALSE; /* not done yet */

  /* Start the first command in the DO phase */
  result = pop3_perform_command(data);
  if(result)
    return result;

  /* Run the state-machine */
  result = pop3_multi_statemach(data, dophase_done);
  *connected = conn->bits.tcpconnect[FIRSTSOCKET];

  if(*dophase_done)
    DEBUGF(infof(data, "DO phase is complete"));

  return result;
}
