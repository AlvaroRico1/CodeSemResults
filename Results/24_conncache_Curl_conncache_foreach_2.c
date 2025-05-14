// Source: curl/lib/conncache.c
// Lines 331-351
bool Curl_conncache_foreach(struct Curl_easy *data,
                            struct conncache *connc,
                            void *param,
                            int (*func)(struct Curl_easy *data,
                                        struct connectdata *conn, void *param))
{
  struct Curl_hash_iterator iter;
  struct Curl_llist_element *curr;
  struct Curl_hash_element *he;

  if(!connc)
    return FALSE;

  CONNCACHE_LOCK(data);
  Curl_hash_start_iterate(&connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    struct connectbundle *bundle;

    bundle = he->ptr;
    he = Curl_hash_next_element(&iter);

    curr = bundle->conn_list.head;
    while(curr) {
      /* Yes, we need to update curr before calling func(), because func()
         might decide to remove the connection */
      struct connectdata *conn = curr->ptr;
      curr = curr->next;

      if(1 == func(data, conn, param)) {
        CONNCACHE_UNLOCK(data);
        return TRUE;
      }
    }
