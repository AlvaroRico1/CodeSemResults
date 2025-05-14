// Source: curl/lib/hsts.c
// Lines 242-243
struct stsentry *Curl_hsts(struct hsts *h, const char *hostname,
                           bool subdomain)
{
  if(h) {
    time_t now = time(NULL);
    size_t hlen = strlen(hostname);
    struct Curl_llist_element *e;
    struct Curl_llist_element *n;
    for(e = h->list.head; e; e = n) {
      struct stsentry *sts = e->ptr;
      n = e->next;
      if(sts->expires <= now) {
        /* remove expired entries */
        Curl_llist_remove(&h->list, &sts->node, NULL);
        hsts_free(sts);
        continue;
      }
      if(subdomain && sts->includeSubDomains) {
        size_t ntail = strlen(sts->host);
        if(ntail < hlen) {
          size_t offs = hlen - ntail;
          if((hostname[offs-1] == '.') &&
             Curl_strncasecompare(&hostname[offs], sts->host, ntail))
            return sts;
        }
      }
      if(Curl_strcasecompare(hostname, sts->host))
        return sts;
    }
  }
