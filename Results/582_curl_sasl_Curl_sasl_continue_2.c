// Source: curl/lib/curl_sasl.c
// Lines 500-511
CURLcode Curl_sasl_continue(struct SASL *sasl, struct Curl_easy *data,
                            struct connectdata *conn,
                            int code, saslprogress *progress)
{
  CURLcode result = CURLE_OK;
  saslstate newstate = SASL_FINAL;
  struct bufref resp;
  const char * const hostname = SSL_HOST_NAME();
  const long int port = SSL_HOST_PORT();
#if !defined(CURL_DISABLE_CRYPTO_AUTH) || defined(USE_KERBEROS5) ||     \
  defined(USE_NTLM)
  const char *service = data->set.str[STRING_SERVICE_NAME] ?
    data->set.str[STRING_SERVICE_NAME] :
    sasl->params->service;
#endif
  const char *oauth_bearer = data->set.str[STRING_BEARER];
  struct bufref serverdata;

  Curl_bufref_init(&serverdata);
  Curl_bufref_init(&resp);
  *progress = SASL_INPROGRESS;

  if(sasl->state == SASL_FINAL) {
    if(code != sasl->params->finalcode)
      result = CURLE_LOGIN_DENIED;
    *progress = SASL_DONE;
    state(sasl, data, SASL_STOP);
    return result;
  }

  if(sasl->state != SASL_CANCEL && sasl->state != SASL_OAUTH2_RESP &&
     code != sasl->params->contcode) {
    *progress = SASL_DONE;
    state(sasl, data, SASL_STOP);
    return CURLE_LOGIN_DENIED;
  }

  switch(sasl->state) {
  case SASL_STOP:
    *progress = SASL_DONE;
    return result;
  case SASL_PLAIN:
    result = Curl_auth_create_plain_message(conn->sasl_authzid,
                                            conn->user, conn->passwd, &resp);
    break;
  case SASL_LOGIN:
    result = Curl_auth_create_login_message(conn->user, &resp);
    newstate = SASL_LOGIN_PASSWD;
    break;
  case SASL_LOGIN_PASSWD:
    result = Curl_auth_create_login_message(conn->passwd, &resp);
    break;
  case SASL_EXTERNAL:
    result = Curl_auth_create_external_message(conn->user, &resp);
    break;
#ifndef CURL_DISABLE_CRYPTO_AUTH
#ifdef USE_GSASL
  case SASL_GSASL:
    result = get_server_message(sasl, data, &serverdata);
    if(!result)
      result = Curl_auth_gsasl_token(data, &serverdata, &conn->gsasl, &resp);
    if(!result && Curl_bufref_len(&resp) > 0)
      newstate = SASL_GSASL;
    break;
#endif
  case SASL_CRAMMD5:
    result = get_server_message(sasl, data, &serverdata);
    if(!result)
      result = Curl_auth_create_cram_md5_message(&serverdata, conn->user,
                                                 conn->passwd, &resp);
    break;
  case SASL_DIGESTMD5:
    result = get_server_message(sasl, data, &serverdata);
    if(!result)
      result = Curl_auth_create_digest_md5_message(data, &serverdata,
                                                   conn->user, conn->passwd,
                                                   service, &resp);
    newstate = SASL_DIGESTMD5_RESP;
    break;
  case SASL_DIGESTMD5_RESP:
    /* Keep response NULL to output an empty line. */
    break;
#endif

#ifdef USE_NTLM
  case SASL_NTLM:
    /* Create the type-1 message */
    result = Curl_auth_create_ntlm_type1_message(data,
                                                 conn->user, conn->passwd,
                                                 service, hostname,
                                                 &conn->ntlm, &resp);
    newstate = SASL_NTLM_TYPE2MSG;
    break;
  case SASL_NTLM_TYPE2MSG:
    /* Decode the type-2 message */
    result = get_server_message(sasl, data, &serverdata);
    if(!result)
      result = Curl_auth_decode_ntlm_type2_message(data, &serverdata,
                                                   &conn->ntlm);
    if(!result)
      result = Curl_auth_create_ntlm_type3_message(data, conn->user,
                                                   conn->passwd, &conn->ntlm,
                                                   &resp);
    break;
#endif

#if defined(USE_KERBEROS5)
  case SASL_GSSAPI:
    result = Curl_auth_create_gssapi_user_message(data, conn->user,
                                                  conn->passwd,
                                                  service,
                                                  conn->host.name,
                                                  sasl->mutual_auth, NULL,
                                                  &conn->krb5,
                                                  &resp);
    newstate = SASL_GSSAPI_TOKEN;
    break;
  case SASL_GSSAPI_TOKEN:
    result = get_server_message(sasl, data, &serverdata);
    if(!result) {
      if(sasl->mutual_auth) {
        /* Decode the user token challenge and create the optional response
           message */
        result = Curl_auth_create_gssapi_user_message(data, NULL, NULL,
                                                      NULL, NULL,
                                                      sasl->mutual_auth,
                                                      &serverdata,
                                                      &conn->krb5,
                                                      &resp);
        newstate = SASL_GSSAPI_NO_DATA;
      }
      else
        /* Decode the security challenge and create the response message */
        result = Curl_auth_create_gssapi_security_message(data,
                                                          conn->sasl_authzid,
                                                          &serverdata,
                                                          &conn->krb5,
                                                          &resp);
    }
    break;
  case SASL_GSSAPI_NO_DATA:
    /* Decode the security challenge and create the response message */
    result = get_server_message(sasl, data, &serverdata);
    if(!result)
      result = Curl_auth_create_gssapi_security_message(data,
                                                        conn->sasl_authzid,
                                                        &serverdata,
                                                        &conn->krb5,
                                                        &resp);
    break;
#endif

  case SASL_OAUTH2:
    /* Create the authorisation message */
    if(sasl->authused == SASL_MECH_OAUTHBEARER) {
      result = Curl_auth_create_oauth_bearer_message(conn->user,
                                                     hostname,
                                                     port,
                                                     oauth_bearer,
                                                     &resp);

      /* Failures maybe sent by the server as continuations for OAUTHBEARER */
      newstate = SASL_OAUTH2_RESP;
    }
    else
      result = Curl_auth_create_xoauth_bearer_message(conn->user,
                                                      oauth_bearer,
                                                      &resp);
    break;

  case SASL_OAUTH2_RESP:
    /* The continuation is optional so check the response code */
    if(code == sasl->params->finalcode) {
      /* Final response was received so we are done */
      *progress = SASL_DONE;
      state(sasl, data, SASL_STOP);
      return result;
    }
    else if(code == sasl->params->contcode) {
      /* Acknowledge the continuation by sending a 0x01 response. */
      Curl_bufref_set(&resp, "\x01", 1, NULL);
      break;
    }
    else {
      *progress = SASL_DONE;
      state(sasl, data, SASL_STOP);
      return CURLE_LOGIN_DENIED;
    }

  case SASL_CANCEL:
    /* Remove the offending mechanism from the supported list */
    sasl->authmechs ^= sasl->authused;

    /* Start an alternative SASL authentication */
    return Curl_sasl_start(sasl, data, conn, sasl->force_ir, progress);
  default:
    failf(data, "Unsupported SASL authentication mechanism");
    result = CURLE_UNSUPPORTED_PROTOCOL;  /* Should not happen */
    break;
  }

  Curl_bufref_free(&serverdata);

  switch(result) {
  case CURLE_BAD_CONTENT_ENCODING:
    /* Cancel dialog */
    result = sasl->params->sendcont(data, conn, "*");
    newstate = SASL_CANCEL;
    break;
  case CURLE_OK:
    result = build_message(data, &resp);
    if(!result)
      result = sasl->params->sendcont(data, conn,
                                      (const char *) Curl_bufref_ptr(&resp));
    break;
  default:
    newstate = SASL_STOP;    /* Stop on error */
    *progress = SASL_DONE;
    break;
  }

  Curl_bufref_free(&resp);

  state(sasl, data, newstate);

  return result;
}
