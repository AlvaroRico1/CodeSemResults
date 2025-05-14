// Source: curl/lib/altsvc.c
// Lines 307-312
void Curl_altsvc_cleanup(struct altsvcinfo **altsvcp)
{
  struct Curl_llist_element *e;
  struct Curl_llist_element *n;
  if(*altsvcp) {
    struct altsvcinfo *altsvc = *altsvcp;
    for(e = altsvc->list.head; e; e = n) {
      struct altsvc *as = e->ptr;
      n = e->next;
      altsvc_free(as);
    }
