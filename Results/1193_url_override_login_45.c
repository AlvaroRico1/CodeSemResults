// Source: curl/lib/url.c
// Lines 2887-2891
static CURLcode override_login(struct Curl_easy *data,
                               struct connectdata *conn)
{
  CURLUcode uc;
  char **userp = &conn->user;
  char **passwdp = &conn->passwd;
  char **optionsp = &conn->options;

#ifndef CURL_DISABLE_NETRC
  if(data->set.use_netrc == CURL_NETRC_REQUIRED && conn->bits.user_passwd) {
    Curl_safefree(*userp);
    Curl_safefree(*passwdp);
    conn->bits.user_passwd = FALSE; /* disable user+password */
  }
#endif

  if(data->set.str[STRING_OPTIONS]) {
    free(*optionsp);
    *optionsp = strdup(data->set.str[STRING_OPTIONS]);
    if(!*optionsp)
      return CURLE_OUT_OF_MEMORY;
  }

#ifndef CURL_DISABLE_NETRC
  conn->bits.netrc = FALSE;
  if(data->set.use_netrc && !data->set.str[STRING_USERNAME]) {
    bool netrc_user_changed = FALSE;
    bool netrc_passwd_changed = FALSE;
    int ret;

    ret = Curl_parsenetrc(conn->host.name,
                          userp, passwdp,
                          &netrc_user_changed, &netrc_passwd_changed,
                          data->set.str[STRING_NETRC_FILE]);
    if(ret > 0) {
      infof(data, "Couldn't find host %s in the %s file; using defaults",
            conn->host.name, data->set.str[STRING_NETRC_FILE]);
    }
    else if(ret < 0) {
      return CURLE_OUT_OF_MEMORY;
    }
    else {
      /* set bits.netrc TRUE to remember that we got the name from a .netrc
         file, so that it is safe to use even if we followed a Location: to a
         different host or similar. */
      conn->bits.netrc = TRUE;
      conn->bits.user_passwd = TRUE; /* enable user+password */
    }
  }
#endif

  /* for updated strings, we update them in the URL */
  if(*userp) {
    CURLcode result = Curl_setstropt(&data->state.aptr.user, *userp);
    if(result)
      return result;
  }
  if(data->state.aptr.user) {
    uc = curl_url_set(data->state.uh, CURLUPART_USER, data->state.aptr.user,
                      CURLU_URLENCODE);
    if(uc)
      return Curl_uc_to_curlcode(uc);
    if(!*userp) {
      *userp = strdup(data->state.aptr.user);
      if(!*userp)
        return CURLE_OUT_OF_MEMORY;
    }
  }

  if(*passwdp) {
    CURLcode result = Curl_setstropt(&data->state.aptr.passwd, *passwdp);
    if(result)
      return result;
  }
  if(data->state.aptr.passwd) {
    uc = curl_url_set(data->state.uh, CURLUPART_PASSWORD,
                      data->state.aptr.passwd, CURLU_URLENCODE);
    if(uc)
      return Curl_uc_to_curlcode(uc);
    if(!*passwdp) {
      *passwdp = strdup(data->state.aptr.passwd);
      if(!*passwdp)
        return CURLE_OUT_OF_MEMORY;
    }
  }

  return CURLE_OK;
}
