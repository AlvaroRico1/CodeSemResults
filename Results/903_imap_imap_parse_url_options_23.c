// Source: curl/lib/imap.c
// Lines 1908-1912
static CURLcode imap_parse_url_options(struct connectdata *conn)
{
  CURLcode result = CURLE_OK;
  struct imap_conn *imapc = &conn->proto.imapc;
  const char *ptr = conn->options;

  imapc->sasl.resetprefs = TRUE;

  while(!result && ptr && *ptr) {
    const char *key = ptr;
    const char *value;

    while(*ptr && *ptr != '=')
        ptr++;

    value = ptr + 1;

    while(*ptr && *ptr != ';')
      ptr++;

    if(strncasecompare(key, "AUTH=", 5))
      result = Curl_sasl_parse_url_auth_option(&imapc->sasl,
                                               value, ptr - value);
    else
      result = CURLE_URL_MALFORMAT;

    if(*ptr == ';')
      ptr++;
  }

  switch(imapc->sasl.prefmech) {
  case SASL_AUTH_NONE:
    imapc->preftype = IMAP_TYPE_NONE;
    break;
  case SASL_AUTH_DEFAULT:
    imapc->preftype = IMAP_TYPE_ANY;
    break;
  default:
    imapc->preftype = IMAP_TYPE_SASL;
    break;
  }

  return result;
}
