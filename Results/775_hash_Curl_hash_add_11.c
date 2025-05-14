// Source: curl/lib/hash.c
// Lines 112-112
Curl_hash_add(struct Curl_hash *h, void *key, size_t key_len, void *p)
{
  struct Curl_hash_element  *he;
  struct Curl_llist_element *le;
  struct Curl_llist *l = FETCH_LIST(h, key, key_len);

  for(le = l->head; le; le = le->next) {
    he = (struct Curl_hash_element *) le->ptr;
    if(h->comp_func(he->key, he->key_len, key, key_len)) {
      Curl_llist_remove(l, le, (void *)h);
      --h->size;
      break;
    }
  }

  he = mk_hash_element(key, key_len, p);
  if(he) {
    Curl_llist_insert_next(l, l->tail, he, &he->list);
    ++h->size;
    return p; /* return the new entry */
  }

  return NULL; /* failure */
}
