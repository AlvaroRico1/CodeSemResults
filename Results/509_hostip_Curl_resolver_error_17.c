// Source: curl/lib/hostip.c
// Lines 1268-1274
CURLcode Curl_resolver_error(struct Curl_easy *data)
{
  const char *host_or_proxy;
  CURLcode result;

#ifndef CURL_DISABLE_PROXY
  struct connectdata *conn = data->conn;
  if(conn->bits.httpproxy) {
    host_or_proxy = "proxy";
    result = CURLE_COULDNT_RESOLVE_PROXY;
  }
  else
#endif
  {
    host_or_proxy = "host";
    result = CURLE_COULDNT_RESOLVE_HOST;
  }

  failf(data, "Could not resolve %s: %s", host_or_proxy,
        data->state.async.hostname);

  return result;
}
