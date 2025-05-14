// Source: curl/src/tool_operate.c
// Lines 2117-2124
static CURLcode add_parallel_transfers(struct GlobalConfig *global,
                                       CURLM *multi,
                                       CURLSH *share,
                                       bool *morep,
                                       bool *addedp)
{
  struct per_transfer *per;
  CURLcode result = CURLE_OK;
  CURLMcode mcode;
  bool sleeping = FALSE;
  *addedp = FALSE;
  *morep = FALSE;
  result = create_transfer(global, share, addedp);
  if(result)
    return result;
  for(per = transfers; per && (all_added < global->parallel_max);
      per = per->next) {
    bool getadded = FALSE;
    if(per->added)
      /* already added */
      continue;
    if(per->startat && (time(NULL) < per->startat)) {
      /* this is still delaying */
      sleeping = TRUE;
      continue;
    }

    result = pre_transfer(global, per);
    if(result)
      return result;

    /* parallel connect means that we don't set PIPEWAIT since pipewait
       will make libcurl prefer multiplexing */
    (void)curl_easy_setopt(per->curl, CURLOPT_PIPEWAIT,
                           global->parallel_connect ? 0L : 1L);
    (void)curl_easy_setopt(per->curl, CURLOPT_PRIVATE, per);
    (void)curl_easy_setopt(per->curl, CURLOPT_XFERINFOFUNCTION, xferinfo_cb);
    (void)curl_easy_setopt(per->curl, CURLOPT_XFERINFODATA, per);
    (void)curl_easy_setopt(per->curl, CURLOPT_NOPROGRESS, 0L);

    mcode = curl_multi_add_handle(multi, per->curl);
    if(mcode)
      return CURLE_OUT_OF_MEMORY;

    result = create_transfer(global, share, &getadded);
    if(result)
      return result;
    per->added = TRUE;
    all_added++;
    *addedp = TRUE;
  }
