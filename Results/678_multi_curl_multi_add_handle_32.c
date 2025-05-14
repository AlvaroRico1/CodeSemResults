// Source: curl/lib/multi.c
// Lines 425-484
CURLMcode curl_multi_add_handle(struct Curl_multi *multi,
                                struct Curl_easy *data)
{
  /* First, make some basic checks that the CURLM handle is a good handle */
  if(!GOOD_MULTI_HANDLE(multi))
    return CURLM_BAD_HANDLE;

  /* Verify that we got a somewhat good easy handle too */
  if(!GOOD_EASY_HANDLE(data))
    return CURLM_BAD_EASY_HANDLE;

  /* Prevent users from adding same easy handle more than once and prevent
     adding to more than one multi stack */
  if(data->multi)
    return CURLM_ADDED_ALREADY;

  if(multi->in_callback)
    return CURLM_RECURSIVE_API_CALL;

  /* Initialize timeout list for this handle */
  Curl_llist_init(&data->state.timeoutlist, NULL);

  /*
   * No failure allowed in this function beyond this point. And no
   * modification of easy nor multi handle allowed before this except for
   * potential multi's connection cache growing which won't be undone in this
   * function no matter what.
   */
  if(data->set.errorbuffer)
    data->set.errorbuffer[0] = 0;

  /* set the easy handle */
  multistate(data, MSTATE_INIT);

  /* for multi interface connections, we share DNS cache automatically if the
     easy handle's one is currently not set. */
  if(!data->dns.hostcache ||
     (data->dns.hostcachetype == HCACHE_NONE)) {
    data->dns.hostcache = &multi->hostcache;
    data->dns.hostcachetype = HCACHE_MULTI;
  }

  /* Point to the shared or multi handle connection cache */
  if(data->share && (data->share->specifier & (1<< CURL_LOCK_DATA_CONNECT)))
    data->state.conn_cache = &data->share->conn_cache;
  else
    data->state.conn_cache = &multi->conn_cache;
  data->state.lastconnect_id = -1;

#ifdef USE_LIBPSL
  /* Do the same for PSL. */
  if(data->share && (data->share->specifier & (1 << CURL_LOCK_DATA_PSL)))
    data->psl = &data->share->psl;
  else
    data->psl = &multi->psl;
#endif

  /* We add the new entry last in the list. */
  data->next = NULL; /* end of the line */
  if(multi->easyp) {
    struct Curl_easy *last = multi->easylp;
    last->next = data;
    data->prev = last;
    multi->easylp = data; /* the new last node */
  }
