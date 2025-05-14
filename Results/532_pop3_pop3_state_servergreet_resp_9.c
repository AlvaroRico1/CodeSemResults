// Source: curl/lib/pop3.c
// Lines 621-626
static CURLcode pop3_state_servergreet_resp(struct Curl_easy *data,
                                            int pop3code,
                                            pop3state instate)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct pop3_conn *pop3c = &conn->proto.pop3c;
  const char *line = data->state.buffer;
  size_t len = strlen(line);

  (void)instate; /* no use for this yet */

  if(pop3code != '+') {
    failf(data, "Got unexpected pop3-server response");
    result = CURLE_WEIRD_SERVER_REPLY;
  }
  else {
    /* Does the server support APOP authentication? */
    if(len >= 4 && line[len - 2] == '>') {
      /* Look for the APOP timestamp */
      size_t i;
      for(i = 3; i < len - 2; ++i) {
        if(line[i] == '<') {
          /* Calculate the length of the timestamp */
          size_t timestamplen = len - 1 - i;
          char *at;
          if(!timestamplen)
            break;

          /* Allocate some memory for the timestamp */
          pop3c->apoptimestamp = (char *)calloc(1, timestamplen + 1);

          if(!pop3c->apoptimestamp)
            break;

          /* Copy the timestamp */
          memcpy(pop3c->apoptimestamp, line + i, timestamplen);
          pop3c->apoptimestamp[timestamplen] = '\0';

          /* If the timestamp does not contain '@' it is not (as required by
             RFC-1939) conformant to the RFC-822 message id syntax, and we
             therefore do not use APOP authentication. */
          at = strchr(pop3c->apoptimestamp, '@');
          if(!at)
            Curl_safefree(pop3c->apoptimestamp);
          else
            /* Store the APOP capability */
            pop3c->authtypes |= POP3_TYPE_APOP;
          break;
        }
      }
    }

    result = pop3_perform_capa(data, conn);
  }

  return result;
}
