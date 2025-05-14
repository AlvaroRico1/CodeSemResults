// Source: curl/src/tool_cb_rea.c
// Lines 65-66
int tool_readbusy_cb(void *clientp,
                     curl_off_t dltotal, curl_off_t dlnow,
                     curl_off_t ultotal, curl_off_t ulnow)
{
  struct per_transfer *per = clientp;
  struct OperationConfig *config = per->config;

  (void)dltotal;  /* unused */
  (void)dlnow;  /* unused */
  (void)ultotal;  /* unused */
  (void)ulnow;  /* unused */

  if(config->readbusy) {
    config->readbusy = FALSE;
    curl_easy_pause(per->curl, CURLPAUSE_CONT);
  }

  return per->noprogress? 0 : CURL_PROGRESSFUNC_CONTINUE;
}
