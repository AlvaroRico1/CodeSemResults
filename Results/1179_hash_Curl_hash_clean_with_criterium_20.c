// Source: curl/lib/hash.c
// Lines 233-235
Curl_hash_clean_with_criterium(struct Curl_hash *h, void *user,
                               int (*comp)(void *, void *))
{
  struct Curl_llist_element *le;
  struct Curl_llist_element *lnext;
  struct Curl_llist *list;
  int i;

  if(!h)
    return;

  for(i = 0; i < h->slots; ++i) {
    list = &h->table[i];
    le = list->head; /* get first list entry */
    while(le) {
      struct Curl_hash_element *he = le->ptr;
      lnext = le->next;
      /* ask the callback function if we shall remove this entry or not */
      if(!comp || comp(user, he->ptr)) {
        Curl_llist_remove(list, le, (void *) h);
        --h->size; /* one less entry in the hash now */
      }
      le = lnext;
    }
  }
}
