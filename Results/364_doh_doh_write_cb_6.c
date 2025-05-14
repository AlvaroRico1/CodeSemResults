// Source: curl/lib/doh.c
// Lines 178-181
doh_write_cb(const void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct dynbuf *mem = (struct dynbuf *)userp;

  if(Curl_dyn_addn(mem, contents, realsize))
    return 0;

  return realsize;
}
