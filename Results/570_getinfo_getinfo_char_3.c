static CURLcode getinfo_char(struct Curl_easy *data, CURLINFO info,
                             const char **param_charp)
{
  switch(info) {
  case CURLINFO_EFFECTIVE_URL:
    *param_charp = data->state.url?data->state.url:(char *)"";
    break;
  case CURLINFO_EFFECTIVE_METHOD: {
    const char *m = data->set.str[STRING_CUSTOMREQUEST];
    if(!m) {
      if(data->set.opt_no_body)
        m = "HEAD";
#ifndef CURL_DISABLE_HTTP
      else {
        switch(data->state.httpreq) {
        case HTTPREQ_POST:
        case HTTPREQ_POST_FORM:
        case HTTPREQ_POST_MIME:
          m = "POST";
          break;
        case HTTPREQ_PUT:
          m = "PUT";
          break;
        default: /* this should never happen */
        case HTTPREQ_GET:
          m = "GET";
          break;
        case HTTPREQ_HEAD:
          m = "HEAD";
          break;
        }
      }
#endif
    }
    *param_charp = m;
  }


// Source: getinfo.c
// Lines 92-127
