// Source: curl/lib/multi.c
// Lines 3271-3272
multi_deltimeout(struct Curl_easy *data, expire_id eid)
{
  struct Curl_llist_element *e;
  struct Curl_llist *timeoutlist = &data->state.timeoutlist;
  /* find and remove the specific node from the list */
  for(e = timeoutlist->head; e; e = e->next) {
    struct time_node *n = (struct time_node *)e->ptr;
    if(n->eid == eid) {
      Curl_llist_remove(timeoutlist, e, NULL);
      return;
    }
  }
}
