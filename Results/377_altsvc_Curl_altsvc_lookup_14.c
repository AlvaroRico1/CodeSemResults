// Source: curl/lib/altsvc.c
// Lines 619-620
bool Curl_altsvc_lookup(struct altsvcinfo *asi,
                        enum alpnid srcalpnid, const char *srchost,
                        int srcport,
                        struct altsvc **dstentry,
                        const int versions) /* one or more bits */
{
  struct Curl_llist_element *e;
  struct Curl_llist_element *n;
  time_t now = time(NULL);
  DEBUGASSERT(asi);
  DEBUGASSERT(srchost);
  DEBUGASSERT(dstentry);

  for(e = asi->list.head; e; e = n) {
    struct altsvc *as = e->ptr;
    n = e->next;
    if(as->expires < now) {
      /* an expired entry, remove */
      Curl_llist_remove(&asi->list, e, NULL);
      altsvc_free(as);
      continue;
    }
    if((as->src.alpnid == srcalpnid) &&
       strcasecompare(as->src.host, srchost) &&
       (as->src.port == srcport) &&
       (versions & as->dst.alpnid)) {
      /* match */
      *dstentry = as;
      return TRUE;
    }
  }
  return FALSE;
}
