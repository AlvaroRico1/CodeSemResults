// Source: curl/lib/hsts.c
// Lines 398-408
static CURLcode hsts_add(struct hsts *h, char *line)
{
  /* Example lines:
     example.com "20191231 10:00:00"
     .example.net "20191231 10:00:00"
   */
  char host[MAX_HSTS_HOSTLEN + 1];
  char date[MAX_HSTS_DATELEN + 1];
  int rc;

  rc = sscanf(line,
              "%" MAX_HSTS_HOSTLENSTR "s \"%" MAX_HSTS_DATELENSTR "[^\"]\"",
              host, date);
  if(2 == rc) {
    time_t expires = Curl_getdate_capped(date);
    CURLcode result;
    char *p = host;
    bool subdomain = FALSE;
    if(p[0] == '.') {
      p++;
      subdomain = TRUE;
    }
    result = hsts_create(h, p, subdomain, expires);
    if(result)
      return result;
  }
