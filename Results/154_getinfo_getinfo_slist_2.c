// Source: curl/lib/getinfo.c
// Lines 482-487
static CURLcode getinfo_slist(struct Curl_easy *data, CURLINFO info,
                              struct curl_slist **param_slistp)
{
  union {
    struct curl_certinfo *to_certinfo;
    struct curl_slist    *to_slist;
  } ptr;
