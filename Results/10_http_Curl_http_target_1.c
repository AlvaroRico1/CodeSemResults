CURLcode Curl_http_target(struct Curl_easy *data,
                          struct connectdata *conn,
                          struct dynbuf *r)
{
  CURLcode result = CURLE_OK;
  const char *path = data->state.up.path;
  const char *query = data->state.up.query;

  if(data->set.str[STRING_TARGET]) {
    path = data->set.str[STRING_TARGET];
    query = NULL;
  }

#ifndef CURL_DISABLE_PROXY
  if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
    /* Using a proxy but does not tunnel through it */

    /* The path sent to the proxy is in fact the entire URL. But if the remote
       host is a IDN-name, we must make sure that the request we produce only
       uses the encoded host name! */

    /* and no fragment part */
    CURLUcode uc;
    char *url;
    CURLU *h = curl_url_dup(data->state.uh);
    if(!h)
      return CURLE_OUT_OF_MEMORY;

    if(conn->host.dispname != conn->host.name) {
      uc = curl_url_set(h, CURLUPART_HOST, conn->host.name, 0);
      if(uc) {
        curl_url_cleanup(h);
        return CURLE_OUT_OF_MEMORY;
      }
    }
    uc = curl_url_set(h, CURLUPART_FRAGMENT, NULL, 0);
    if(uc) {
      curl_url_cleanup(h);
      return CURLE_OUT_OF_MEMORY;
    }

    if(strcasecompare("http", data->state.up.scheme)) {
      /* when getting HTTP, we don't want the userinfo the URL */
      uc = curl_url_set(h, CURLUPART_USER, NULL, 0);
      if(uc) {
        curl_url_cleanup(h);
        return CURLE_OUT_OF_MEMORY;
      }
      uc = curl_url_set(h, CURLUPART_PASSWORD, NULL, 0);
      if(uc) {
        curl_url_cleanup(h);
        return CURLE_OUT_OF_MEMORY;
      }
    }
    /* Extract the URL to use in the request. Store in STRING_TEMP_URL for
       clean-up reasons if the function returns before the free() further
       down. */
    uc = curl_url_get(h, CURLUPART_URL, &url, CURLU_NO_DEFAULT_PORT);
    if(uc) {
      curl_url_cleanup(h);
      return CURLE_OUT_OF_MEMORY;
    }

    curl_url_cleanup(h);

    /* target or url */
    result = Curl_dyn_add(r, data->set.str[STRING_TARGET]?
      data->set.str[STRING_TARGET]:url);
    free(url);
    if(result)
      return (result);

    if(strcasecompare("ftp", data->state.up.scheme)) {
      if(data->set.proxy_transfer_mode) {
        /* when doing ftp, append ;type=<a|i> if not present */
        char *type = strstr(path, ";type=");
        if(type && type[6] && type[7] == 0) {
          switch(Curl_raw_toupper(type[6])) {
          case 'A':
          case 'D':
          case 'I':
            break;
          default:
            type = NULL;
          }
        }
        if(!type) {
          result = Curl_dyn_addf(r, ";type=%c",
                                 data->state.prefer_ascii ? 'a' : 'i');
          if(result)
            return result;
        }
      }


// Source: http.c
// Lines 2173-2265
