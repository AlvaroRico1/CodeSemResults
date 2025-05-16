CURLcode Curl_http_cookies(struct Curl_easy *data,
                           struct connectdata *conn,
                           struct dynbuf *r)
{
  CURLcode result = CURLE_OK;
  char *addcookies = NULL;
  if(data->set.str[STRING_COOKIE] && !Curl_checkheaders(data, "Cookie"))
    addcookies = data->set.str[STRING_COOKIE];

  if(data->cookies || addcookies) {
    struct Cookie *co = NULL; /* no cookies from start */
    int count = 0;

    if(data->cookies && data->state.cookie_engine) {
      const char *host = data->state.aptr.cookiehost ?
        data->state.aptr.cookiehost : conn->host.name;
      const bool secure_context =
        conn->handler->protocol&CURLPROTO_HTTPS ||
        strcasecompare("localhost", host) ||
        !strcmp(host, "127.0.0.1") ||
        !strcmp(host, "[::1]") ? TRUE : FALSE;
      Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
      co = Curl_cookie_getlist(data->cookies, host, data->state.up.path,
                               secure_context);
      Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
    }
    if(co) {
      struct Cookie *store = co;
      /* now loop through all cookies that matched */
      while(co) {
        if(co->value) {
          if(0 == count) {
            result = Curl_dyn_add(r, "Cookie: ");
            if(result)
              break;
          }
          result = Curl_dyn_addf(r, "%s%s=%s", count?"; ":"",
                                 co->name, co->value);
          if(result)
            break;
          count++;
        }
        co = co->next; /* next cookie please */
      }
      Curl_cookie_freelist(store);
    }
    if(addcookies && !result) {
      if(!count)
        result = Curl_dyn_add(r, "Cookie: ");
      if(!result) {
        result = Curl_dyn_addf(r, "%s%s", count?"; ":"", addcookies);
        count++;
      }
    }
    if(count && !result)
      result = Curl_dyn_add(r, "\r\n");

    if(result)
      return result;
  }


// Source: http.c
// Lines 2700-2759
