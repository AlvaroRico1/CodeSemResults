static void altsvc_flush(struct altsvcinfo *asi, enum alpnid srcalpnid,
                         const char *srchost, unsigned short srcport)
{
  struct Curl_llist_element *e;
  struct Curl_llist_element *n;
  for(e = asi->list.head; e; e = n) {
    struct altsvc *as = e->ptr;
    n = e->next;
    if((srcalpnid == as->src.alpnid) &&
       (srcport == as->src.port) &&
       strcasecompare(srchost, as->src.host)) {
      Curl_llist_remove(&asi->list, e, NULL);
      altsvc_free(as);
    }
  }
}


// Source: altsvc.c
// Lines 401-416
