// Source: curl/lib/conncache.c
// Lines 234-238
CURLcode Curl_conncache_add_conn(struct Curl_easy *data)
{
  CURLcode result = CURLE_OK;
  struct connectbundle *bundle = NULL;
  struct connectdata *conn = data->conn;
  struct conncache *connc = data->state.conn_cache;
  DEBUGASSERT(conn);

  /* *find_bundle() locks the connection cache */
  bundle = Curl_conncache_find_bundle(data, conn, data->state.conn_cache,
                                      NULL);
  if(!bundle) {
    int rc;
    char key[HASHKEY_SIZE];

    result = bundle_create(&bundle);
    if(result) {
      goto unlock;
    }

    hashkey(conn, key, sizeof(key), NULL);
    rc = conncache_add_bundle(data->state.conn_cache, key, bundle);

    if(!rc) {
      bundle_destroy(bundle);
      result = CURLE_OUT_OF_MEMORY;
      goto unlock;
    }
  }

  bundle_add_conn(bundle, conn);
  conn->connection_id = connc->next_connection_id++;
  connc->num_conn++;

  DEBUGF(infof(data, "Added connection %ld. "
               "The cache now contains %zu members",
               conn->connection_id, connc->num_conn));

  unlock:
  CONNCACHE_UNLOCK(data);

  return result;
}
