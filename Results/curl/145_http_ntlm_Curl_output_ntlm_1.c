CURLcode Curl_output_ntlm(struct Curl_easy *data, bool proxy)
{
  char *base64 = NULL;
  size_t len = 0;
  CURLcode result = CURLE_OK;
  struct bufref ntlmmsg;

  /* point to the address of the pointer that holds the string to send to the
     server, which is for a plain host or for a HTTP proxy */
  char **allocuserpwd;

  /* point to the username, password, service and host */
  const char *userp;
  const char *passwdp;
  const char *service = NULL;
  const char *hostname = NULL;

  /* point to the correct struct with this */
  struct ntlmdata *ntlm;
  curlntlm *state;
  struct auth *authp;
  struct connectdata *conn = data->conn;

  DEBUGASSERT(conn);
  DEBUGASSERT(data);

  if(proxy) {
#ifndef CURL_DISABLE_PROXY
    allocuserpwd = &data->state.aptr.proxyuserpwd;
    userp = data->state.aptr.proxyuser;
    passwdp = data->state.aptr.proxypasswd;
    service = data->set.str[STRING_PROXY_SERVICE_NAME] ?
      data->set.str[STRING_PROXY_SERVICE_NAME] : "HTTP";
    hostname = conn->http_proxy.host.name;
    ntlm = &conn->proxyntlm;
    state = &conn->proxy_ntlm_state;
    authp = &data->state.authproxy;
#else
    return CURLE_NOT_BUILT_IN;
#endif
  }
  else {
    allocuserpwd = &data->state.aptr.userpwd;
    userp = data->state.aptr.user;
    passwdp = data->state.aptr.passwd;
    service = data->set.str[STRING_SERVICE_NAME] ?
      data->set.str[STRING_SERVICE_NAME] : "HTTP";
    hostname = conn->host.name;
    ntlm = &conn->ntlm;
    state = &conn->http_ntlm_state;
    authp = &data->state.authhost;
  }
  authp->done = FALSE;

  /* not set means empty */
  if(!userp)
    userp = "";

  if(!passwdp)
    passwdp = "";

#ifdef USE_WINDOWS_SSPI
  if(!s_hSecDll) {
    /* not thread safe and leaks - use curl_global_init() to avoid */
    CURLcode err = Curl_sspi_global_init();
    if(!s_hSecDll)
      return err;
  }
#ifdef SECPKG_ATTR_ENDPOINT_BINDINGS
  ntlm->sslContext = conn->sslContext;
#endif
#endif

  Curl_bufref_init(&ntlmmsg);
  switch(*state) {
  case NTLMSTATE_TYPE1:
  default: /* for the weird cases we (re)start here */
    /* Create a type-1 message */
    result = Curl_auth_create_ntlm_type1_message(data, userp, passwdp,
                                                 service, hostname,
                                                 ntlm, &ntlmmsg);
    if(!result) {
      DEBUGASSERT(Curl_bufref_len(&ntlmmsg) != 0);
      result = Curl_base64_encode(data,
                                  (const char *) Curl_bufref_ptr(&ntlmmsg),
                                  Curl_bufref_len(&ntlmmsg), &base64, &len);
      if(!result) {
        free(*allocuserpwd);
        *allocuserpwd = aprintf("%sAuthorization: NTLM %s\r\n",
                                proxy ? "Proxy-" : "",
                                base64);
        free(base64);
        if(!*allocuserpwd)
          result = CURLE_OUT_OF_MEMORY;
      }
    }
    break;

  case NTLMSTATE_TYPE2:
    /* We already received the type-2 message, create a type-3 message */
    result = Curl_auth_create_ntlm_type3_message(data, userp, passwdp,
                                                 ntlm, &ntlmmsg);
    if(!result && Curl_bufref_len(&ntlmmsg)) {
      result = Curl_base64_encode(data,
                                  (const char *) Curl_bufref_ptr(&ntlmmsg),
                                  Curl_bufref_len(&ntlmmsg), &base64, &len);
      if(!result) {
        free(*allocuserpwd);
        *allocuserpwd = aprintf("%sAuthorization: NTLM %s\r\n",
                                proxy ? "Proxy-" : "",
                                base64);
        free(base64);
        if(!*allocuserpwd)
          result = CURLE_OUT_OF_MEMORY;
        else {
          *state = NTLMSTATE_TYPE3; /* we send a type-3 */
          authp->done = TRUE;
        }
      }
    }
    break;

  case NTLMSTATE_TYPE3:
    /* connection is already authenticated,
     * don't send a header in future requests */
    *state = NTLMSTATE_LAST;
    /* FALLTHROUGH */
  case NTLMSTATE_LAST:
    Curl_safefree(*allocuserpwd);
    authp->done = TRUE;
    break;
  }
  Curl_bufref_free(&ntlmmsg);

  return result;
}


// Source: http_ntlm.c
// Lines 127-262
