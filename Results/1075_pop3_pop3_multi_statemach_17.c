// Source: curl/lib/pop3.c
// Lines 1026-1029
static CURLcode pop3_multi_statemach(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct pop3_conn *pop3c = &conn->proto.pop3c;

  if((conn->handler->flags & PROTOPT_SSL) && !pop3c->ssldone) {
    result = Curl_ssl_connect_nonblocking(data, conn, FALSE,
                                          FIRSTSOCKET, &pop3c->ssldone);
    if(result || !pop3c->ssldone)
      return result;
  }

  result = Curl_pp_statemach(data, &pop3c->pp, FALSE, FALSE);
  *done = (pop3c->state == POP3_STOP) ? TRUE : FALSE;

  return result;
}
