// Source: curl/lib/hostip.c
// Lines 199-201
hostcache_timestamp_remove(void *datap, void *hc)
{
  struct hostcache_prune_data *data =
    (struct hostcache_prune_data *) datap;
  struct Curl_dns_entry *c = (struct Curl_dns_entry *) hc;

  return (0 != c->timestamp)
    && (data->now - c->timestamp >= data->cache_timeout);
}
