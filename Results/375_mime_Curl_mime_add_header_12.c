// Source: curl/lib/mime.c
// Lines 1714-1716
CURLcode Curl_mime_add_header(struct curl_slist **slp, const char *fmt, ...)
{
  struct curl_slist *hdr = NULL;
  char *s = NULL;
  va_list ap;

  va_start(ap, fmt);
  s = curl_mvaprintf(fmt, ap);
  va_end(ap);

  if(s) {
    hdr = Curl_slist_append_nodup(*slp, s);
    if(hdr)
      *slp = hdr;
    else
      free(s);
  }

  return hdr? CURLE_OK: CURLE_OUT_OF_MEMORY;
}
