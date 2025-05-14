// Source: curl/lib/multi.c
// Lines 2599-2600
CURLMcode curl_multi_cleanup(struct Curl_multi *multi)
{
  struct Curl_easy *data;
  struct Curl_easy *nextdata;

  if(GOOD_MULTI_HANDLE(multi)) {
    if(multi->in_callback)
      return CURLM_RECURSIVE_API_CALL;

    multi->magic = 0; /* not good anymore */

    /* Firsrt remove all remaining easy handles */
    data = multi->easyp;
    while(data) {
      nextdata = data->next;
      if(!data->state.done && data->conn)
        /* if DONE was never called for this handle */
        (void)multi_done(data, CURLE_OK, TRUE);
      if(data->dns.hostcachetype == HCACHE_MULTI) {
        /* clear out the usage of the shared DNS cache */
        Curl_hostcache_clean(data, data->dns.hostcache);
        data->dns.hostcache = NULL;
        data->dns.hostcachetype = HCACHE_NONE;
      }

      /* Clear the pointer to the connection cache */
      data->state.conn_cache = NULL;
      data->multi = NULL; /* clear the association */

#ifdef USE_LIBPSL
      if(data->psl == &multi->psl)
        data->psl = NULL;
#endif

      data = nextdata;
    }

    /* Close all the connections in the connection cache */
    Curl_conncache_close_all_connections(&multi->conn_cache);

    Curl_hash_destroy(&multi->sockhash);
    Curl_conncache_destroy(&multi->conn_cache);
    Curl_llist_destroy(&multi->msglist, NULL);
    Curl_llist_destroy(&multi->pending, NULL);

    Curl_hash_destroy(&multi->hostcache);
    Curl_psl_destroy(&multi->psl);

#ifdef USE_WINSOCK
    WSACloseEvent(multi->wsa_event);
#else
#ifdef ENABLE_WAKEUP
    sclose(multi->wakeup_pair[0]);
    sclose(multi->wakeup_pair[1]);
#endif
#endif
    free(multi);

    return CURLM_OK;
  }
  return CURLM_BAD_HANDLE;
}
