// Source: curl/lib/url.c
// Lines 1060-1072
static void prune_dead_connections(struct Curl_easy *data)
{
  struct curltime now = Curl_now();
  timediff_t elapsed;

  DEBUGASSERT(!data->conn); /* no connection */
  CONNCACHE_LOCK(data);
  elapsed =
    Curl_timediff(now, data->state.conn_cache->last_cleanup);
  CONNCACHE_UNLOCK(data);

  if(elapsed >= 1000L) {
    struct prunedead prune;
    prune.data = data;
    prune.extracted = NULL;
    while(Curl_conncache_foreach(data, data->state.conn_cache, &prune,
                                 call_extract_if_dead)) {
      /* unlocked */

      /* remove connection from cache */
      Curl_conncache_remove_conn(data, prune.extracted, TRUE);

      /* disconnect it */
      (void)Curl_disconnect(data, prune.extracted, TRUE);
    }
    CONNCACHE_LOCK(data);
    data->state.conn_cache->last_cleanup = now;
    CONNCACHE_UNLOCK(data);
  }
