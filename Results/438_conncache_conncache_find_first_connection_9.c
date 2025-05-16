conncache_find_first_connection(struct conncache *connc)
{
  struct Curl_hash_iterator iter;
  struct Curl_hash_element *he;
  struct connectbundle *bundle;

  Curl_hash_start_iterate(&connc->hash, &iter);

  he = Curl_hash_next_element(&iter);
  while(he) {
    struct Curl_llist_element *curr;
    bundle = he->ptr;

    curr = bundle->conn_list.head;
    if(curr) {
      return curr->ptr;
    }

    he = Curl_hash_next_element(&iter);
  }

  return NULL;
}


// Source: conncache.c
// Lines 371-393
