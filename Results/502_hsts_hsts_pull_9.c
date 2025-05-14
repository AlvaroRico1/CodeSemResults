// Source: curl/lib/hsts.c
// Lines 433-434
static CURLcode hsts_pull(struct Curl_easy *data, struct hsts *h)
{
  /* if the HSTS read callback is set, use it */
  if(data->set.hsts_read) {
    CURLSTScode sc;
    DEBUGASSERT(h);
    do {
      char buffer[257];
      struct curl_hstsentry e;
      e.name = buffer;
      e.namelen = sizeof(buffer)-1;
      e.includeSubDomains = FALSE; /* default */
      e.expire[0] = 0;
      e.name[0] = 0; /* just to make it clean */
      sc = data->set.hsts_read(data, &e, data->set.hsts_read_userp);
      if(sc == CURLSTS_OK) {
        time_t expires;
        CURLcode result;
        if(!e.name[0])
          /* bail out if no name was stored */
          return CURLE_BAD_FUNCTION_ARGUMENT;
        if(e.expire[0])
          expires = Curl_getdate_capped(e.expire);
        else
          expires = TIME_T_MAX; /* the end of time */
        result = hsts_create(h, e.name,
                             /* bitfield to bool conversion: */
                             e.includeSubDomains ? TRUE : FALSE,
                             expires);
        if(result)
          return result;
      }
      else if(sc == CURLSTS_FAIL)
        return CURLE_BAD_FUNCTION_ARGUMENT;
    } while(sc == CURLSTS_OK);
