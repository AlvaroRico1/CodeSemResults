static CURLMcode add_next_timeout(struct curltime now,
                                  struct Curl_multi *multi,
                                  struct Curl_easy *d)
{
  struct curltime *tv = &d->state.expiretime;
  struct Curl_llist *list = &d->state.timeoutlist;
  struct Curl_llist_element *e;
  struct time_node *node = NULL;

  /* move over the timeout list for this specific handle and remove all
     timeouts that are now passed tense and store the next pending
     timeout in *tv */
  for(e = list->head; e;) {
    struct Curl_llist_element *n = e->next;
    timediff_t diff;
    node = (struct time_node *)e->ptr;
    diff = Curl_timediff(node->time, now);
    if(diff <= 0)
      /* remove outdated entry */
      Curl_llist_remove(list, e, NULL);
    else
      /* the list is sorted so get out on the first mismatch */
      break;
    e = n;
  }


// Source: multi.c
// Lines 2905-2929
