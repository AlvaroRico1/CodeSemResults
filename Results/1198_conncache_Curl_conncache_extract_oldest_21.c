// Source: curl/lib/conncache.c
// Lines 487-503
Curl_conncache_extract_oldest(struct Curl_easy *data)
{
  struct conncache *connc = data->state.conn_cache;
  struct Curl_hash_iterator iter;
  struct Curl_llist_element *curr;
  struct Curl_hash_element *he;
  timediff_t highscore =- 1;
  timediff_t score;
  struct curltime now;
  struct connectdata *conn_candidate = NULL;
  struct connectbundle *bundle;
  struct connectbundle *bundle_candidate = NULL;

  now = Curl_now();

  CONNCACHE_LOCK(data);
  Curl_hash_start_iterate(&connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    struct connectdata *conn;

    bundle = he->ptr;

    curr = bundle->conn_list.head;
    while(curr) {
      conn = curr->ptr;

      if(!CONN_INUSE(conn) && !conn->bits.close &&
         !conn->bits.connect_only) {
        /* Set higher score for the age passed since the connection was used */
        score = Curl_timediff(now, conn->lastused);

        if(score > highscore) {
          highscore = score;
          conn_candidate = conn;
          bundle_candidate = bundle;
        }
      }
      curr = curr->next;
    }

    he = Curl_hash_next_element(&iter);
  }
