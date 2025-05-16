static CURLcode parseurlandfillconn(struct Curl_easy *data,
                                    struct connectdata *conn)
{
  CURLcode result;
  CURLU *uh;
  CURLUcode uc;
  char *hostname;
  bool use_set_uh = (data->set.uh && !data->state.this_is_a_follow);

  up_free(data); /* cleanup previous leftovers first */

  /* parse the URL */
  if(use_set_uh) {
    uh = data->state.uh = curl_url_dup(data->set.uh);
  }
  else {
    uh = data->state.uh = curl_url();
  }

  if(!uh)
    return CURLE_OUT_OF_MEMORY;

  if(data->set.str[STRING_DEFAULT_PROTOCOL] &&
     !Curl_is_absolute_url(data->state.url, NULL, MAX_SCHEME_LEN)) {
    char *url = aprintf("%s://%s", data->set.str[STRING_DEFAULT_PROTOCOL],
                        data->state.url);
    if(!url)
      return CURLE_OUT_OF_MEMORY;
    if(data->state.url_alloc)
      free(data->state.url);
    data->state.url = url;
    data->state.url_alloc = TRUE;
  }

  if(!use_set_uh) {
    char *newurl;
    uc = curl_url_set(uh, CURLUPART_URL, data->state.url,
                    CURLU_GUESS_SCHEME |
                    CURLU_NON_SUPPORT_SCHEME |
                    (data->set.disallow_username_in_url ?
                     CURLU_DISALLOW_USER : 0) |
                    (data->set.path_as_is ? CURLU_PATH_AS_IS : 0));
    if(uc) {
      DEBUGF(infof(data, "curl_url_set rejected %s", data->state.url));
      return Curl_uc_to_curlcode(uc);
    }

    /* after it was parsed, get the generated normalized version */
    uc = curl_url_get(uh, CURLUPART_URL, &newurl, 0);
    if(uc)
      return Curl_uc_to_curlcode(uc);
    if(data->state.url_alloc)
      free(data->state.url);
    data->state.url = newurl;
    data->state.url_alloc = TRUE;
  }

  uc = curl_url_get(uh, CURLUPART_SCHEME, &data->state.up.scheme, 0);
  if(uc)
    return Curl_uc_to_curlcode(uc);

  uc = curl_url_get(uh, CURLUPART_HOST, &data->state.up.hostname, 0);
  if(uc) {
    if(!strcasecompare("file", data->state.up.scheme))
      return CURLE_OUT_OF_MEMORY;
  }

#ifndef CURL_DISABLE_HSTS
  if(data->hsts && strcasecompare("http", data->state.up.scheme)) {
    if(Curl_hsts(data->hsts, data->state.up.hostname, TRUE)) {
      char *url;
      Curl_safefree(data->state.up.scheme);
      uc = curl_url_set(uh, CURLUPART_SCHEME, "https", 0);
      if(uc)
        return Curl_uc_to_curlcode(uc);
      if(data->state.url_alloc)
        Curl_safefree(data->state.url);
      /* after update, get the updated version */
      uc = curl_url_get(uh, CURLUPART_URL, &url, 0);
      if(uc)
        return Curl_uc_to_curlcode(uc);
      uc = curl_url_get(uh, CURLUPART_SCHEME, &data->state.up.scheme, 0);
      if(uc) {
        free(url);
        return Curl_uc_to_curlcode(uc);
      }
      data->state.url = url;
      data->state.url_alloc = TRUE;
      infof(data, "Switched from HTTP to HTTPS due to HSTS => %s",
            data->state.url);
    }
  }
#endif

  result = findprotocol(data, conn, data->state.up.scheme);
  if(result)
    return result;

  /*
   * User name and password set with their own options override the
   * credentials possibly set in the URL.
   */
  if(!data->state.aptr.user) {
    /* we don't use the URL API's URL decoder option here since it rejects
       control codes and we want to allow them for some schemes in the user
       and password fields */
    uc = curl_url_get(uh, CURLUPART_USER, &data->state.up.user, 0);
    if(!uc) {
      char *decoded;
      result = Curl_urldecode(NULL, data->state.up.user, 0, &decoded, NULL,
                              conn->handler->flags&PROTOPT_USERPWDCTRL ?
                              REJECT_ZERO : REJECT_CTRL);
      if(result)
        return result;
      conn->user = decoded;
      conn->bits.user_passwd = TRUE;
      result = Curl_setstropt(&data->state.aptr.user, decoded);
      if(result)
        return result;
    }
    else if(uc != CURLUE_NO_USER)
      return Curl_uc_to_curlcode(uc);
  }

  if(!data->state.aptr.passwd) {
    uc = curl_url_get(uh, CURLUPART_PASSWORD, &data->state.up.password, 0);
    if(!uc) {
      char *decoded;
      result = Curl_urldecode(NULL, data->state.up.password, 0, &decoded, NULL,
                              conn->handler->flags&PROTOPT_USERPWDCTRL ?
                              REJECT_ZERO : REJECT_CTRL);
      if(result)
        return result;
      conn->passwd = decoded;
      conn->bits.user_passwd = TRUE;
      result = Curl_setstropt(&data->state.aptr.passwd, decoded);
      if(result)
        return result;
    }


// Source: url.c
// Lines 1914-2052
