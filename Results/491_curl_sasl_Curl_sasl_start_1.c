// Source: curl/lib/curl_sasl.c
// Lines 312-325
CURLcode Curl_sasl_start(struct SASL *sasl, struct Curl_easy *data,
                         struct connectdata *conn,
                         bool force_ir, saslprogress *progress)
{
  CURLcode result = CURLE_OK;
  unsigned int enabledmechs;
  const char *mech = NULL;
  struct bufref resp;
  saslstate state1 = SASL_STOP;
  saslstate state2 = SASL_FINAL;
  const char * const hostname = SSL_HOST_NAME();
  const long int port = SSL_HOST_PORT();
#if defined(USE_KERBEROS5) || defined(USE_NTLM)
  const char *service = data->set.str[STRING_SERVICE_NAME] ?
    data->set.str[STRING_SERVICE_NAME] :
    sasl->params->service;
#endif
  const char *oauth_bearer = data->set.str[STRING_BEARER];
  struct bufref nullmsg;

  Curl_bufref_init(&nullmsg);
  Curl_bufref_init(&resp);
  sasl->force_ir = force_ir;    /* Latch for future use */
  sasl->authused = 0;           /* No mechanism used yet */
  enabledmechs = sasl->authmechs & sasl->prefmech;
  *progress = SASL_IDLE;

  /* Calculate the supported authentication mechanism, by decreasing order of
     security, as well as the initial response where appropriate */
  if((enabledmechs & SASL_MECH_EXTERNAL) && !conn->passwd[0]) {
    mech = SASL_MECH_STRING_EXTERNAL;
    state1 = SASL_EXTERNAL;
    sasl->authused = SASL_MECH_EXTERNAL;

    if(force_ir || data->set.sasl_ir)
      result = Curl_auth_create_external_message(conn->user, &resp);
  }
  else if(conn->bits.user_passwd) {
#if defined(USE_KERBEROS5)
    if((enabledmechs & SASL_MECH_GSSAPI) && Curl_auth_is_gssapi_supported() &&
       Curl_auth_user_contains_domain(conn->user)) {
      sasl->mutual_auth = FALSE;
      mech = SASL_MECH_STRING_GSSAPI;
      state1 = SASL_GSSAPI;
      state2 = SASL_GSSAPI_TOKEN;
      sasl->authused = SASL_MECH_GSSAPI;

      if(force_ir || data->set.sasl_ir)
        result = Curl_auth_create_gssapi_user_message(data, conn->user,
                                                      conn->passwd,
                                                      service,
                                                      conn->host.name,
                                                      sasl->mutual_auth,
                                                      NULL, &conn->krb5,
                                                      &resp);
    }
    else
#endif
#ifdef USE_GSASL
    if((enabledmechs & SASL_MECH_SCRAM_SHA_256) &&
       Curl_auth_gsasl_is_supported(data, SASL_MECH_STRING_SCRAM_SHA_256,
                                    &conn->gsasl)) {
      mech = SASL_MECH_STRING_SCRAM_SHA_256;
      sasl->authused = SASL_MECH_SCRAM_SHA_256;
      state1 = SASL_GSASL;
      state2 = SASL_GSASL;

      result = Curl_auth_gsasl_start(data, conn->user,
                                     conn->passwd, &conn->gsasl);
      if(result == CURLE_OK && (force_ir || data->set.sasl_ir))
        result = Curl_auth_gsasl_token(data, &nullmsg, &conn->gsasl, &resp);
    }
    else if((enabledmechs & SASL_MECH_SCRAM_SHA_1) &&
            Curl_auth_gsasl_is_supported(data, SASL_MECH_STRING_SCRAM_SHA_1,
                                         &conn->gsasl)) {
      mech = SASL_MECH_STRING_SCRAM_SHA_1;
      sasl->authused = SASL_MECH_SCRAM_SHA_1;
      state1 = SASL_GSASL;
      state2 = SASL_GSASL;

      result = Curl_auth_gsasl_start(data, conn->user,
                                     conn->passwd, &conn->gsasl);
      if(result == CURLE_OK && (force_ir || data->set.sasl_ir))
        result = Curl_auth_gsasl_token(data, &nullmsg, &conn->gsasl, &resp);
    }
    else
#endif
#ifndef CURL_DISABLE_CRYPTO_AUTH
    if((enabledmechs & SASL_MECH_DIGEST_MD5) &&
       Curl_auth_is_digest_supported()) {
      mech = SASL_MECH_STRING_DIGEST_MD5;
      state1 = SASL_DIGESTMD5;
      sasl->authused = SASL_MECH_DIGEST_MD5;
    }
    else if(enabledmechs & SASL_MECH_CRAM_MD5) {
      mech = SASL_MECH_STRING_CRAM_MD5;
      state1 = SASL_CRAMMD5;
      sasl->authused = SASL_MECH_CRAM_MD5;
    }
    else
#endif
#ifdef USE_NTLM
    if((enabledmechs & SASL_MECH_NTLM) && Curl_auth_is_ntlm_supported()) {
      mech = SASL_MECH_STRING_NTLM;
      state1 = SASL_NTLM;
      state2 = SASL_NTLM_TYPE2MSG;
      sasl->authused = SASL_MECH_NTLM;

      if(force_ir || data->set.sasl_ir)
        result = Curl_auth_create_ntlm_type1_message(data,
                                                     conn->user, conn->passwd,
                                                     service,
                                                     hostname,
                                                     &conn->ntlm, &resp);
      }
    else
#endif
    if((enabledmechs & SASL_MECH_OAUTHBEARER) && oauth_bearer) {
      mech = SASL_MECH_STRING_OAUTHBEARER;
      state1 = SASL_OAUTH2;
      state2 = SASL_OAUTH2_RESP;
      sasl->authused = SASL_MECH_OAUTHBEARER;

      if(force_ir || data->set.sasl_ir)
        result = Curl_auth_create_oauth_bearer_message(conn->user,
                                                       hostname,
                                                       port,
                                                       oauth_bearer,
                                                       &resp);
    }
    else if((enabledmechs & SASL_MECH_XOAUTH2) && oauth_bearer) {
      mech = SASL_MECH_STRING_XOAUTH2;
      state1 = SASL_OAUTH2;
      sasl->authused = SASL_MECH_XOAUTH2;

      if(force_ir || data->set.sasl_ir)
        result = Curl_auth_create_xoauth_bearer_message(conn->user,
                                                        oauth_bearer,
                                                        &resp);
    }
    else if(enabledmechs & SASL_MECH_PLAIN) {
      mech = SASL_MECH_STRING_PLAIN;
      state1 = SASL_PLAIN;
      sasl->authused = SASL_MECH_PLAIN;

      if(force_ir || data->set.sasl_ir)
        result = Curl_auth_create_plain_message(conn->sasl_authzid,
                                                conn->user, conn->passwd,
                                                &resp);
    }
    else if(enabledmechs & SASL_MECH_LOGIN) {
      mech = SASL_MECH_STRING_LOGIN;
      state1 = SASL_LOGIN;
      state2 = SASL_LOGIN_PASSWD;
      sasl->authused = SASL_MECH_LOGIN;

      if(force_ir || data->set.sasl_ir)
        result = Curl_auth_create_login_message(conn->user, &resp);
    }
  }

  if(!result && mech) {
    if(Curl_bufref_ptr(&resp))
      result = build_message(data, &resp);

    if(sasl->params->maxirlen &&
       strlen(mech) + Curl_bufref_len(&resp) > sasl->params->maxirlen)
      Curl_bufref_free(&resp);

    if(!result)
      result = sasl->params->sendauth(data, conn, mech,
                                      (const char *) Curl_bufref_ptr(&resp));

    if(!result) {
      *progress = SASL_INPROGRESS;
      state(sasl, data, Curl_bufref_ptr(&resp) ? state2 : state1);
    }
  }

  Curl_bufref_free(&resp);
  return result;
}
