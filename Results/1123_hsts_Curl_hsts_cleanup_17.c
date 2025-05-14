// Source: curl/lib/hsts.c
// Lines 92-95
void Curl_hsts_cleanup(struct hsts **hp)
{
  struct hsts *h = *hp;
  if(h) {
    struct Curl_llist_element *e;
    struct Curl_llist_element *n;
    for(e = h->list.head; e; e = n) {
      struct stsentry *sts = e->ptr;
      n = e->next;
      hsts_free(sts);
    }
