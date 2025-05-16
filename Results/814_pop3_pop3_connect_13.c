static CURLcode pop3_connect(struct Curl_easy *data, bool *done)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct pop3_conn *pop3c = &conn->proto.pop3c;
  struct pingpong *pp = &pop3c->pp;

  *done = FALSE; /* default to not done yet */

  /* We always support persistent connections in POP3 */
  connkeep(conn, "POP3 default");

  PINGPONG_SETUP(pp, pop3_statemachine, pop3_endofresp);

  /* Set the default preferred authentication type and mechanism */
  pop3c->preftype = POP3_TYPE_ANY;
  Curl_sasl_init(&pop3c->sasl, &saslpop3);

  /* Initialise the pingpong layer */
  Curl_pp_setup(pp);
  Curl_pp_init(data, pp);

  /* Parse the URL options */
  result = pop3_parse_url_options(conn);
  if(result)
    return result;

  /* Start off waiting for the server greeting response */
  state(data, POP3_SERVERGREET);

  result = pop3_multi_statemach(data, done);

  return result;
}


// Source: pop3.c
// Lines 1089-1122
