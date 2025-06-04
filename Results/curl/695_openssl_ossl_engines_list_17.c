static struct curl_slist *ossl_engines_list(struct Curl_easy *data)
{
  struct curl_slist *list = NULL;
#ifdef USE_OPENSSL_ENGINE
  struct curl_slist *beg;
  ENGINE *e;

  for(e = ENGINE_get_first(); e; e = ENGINE_get_next(e)) {
    beg = curl_slist_append(list, ENGINE_get_id(e));
    if(!beg) {
      curl_slist_free_all(list);
      return NULL;
    }
    list = beg;
  }
#endif
  (void) data;
  return list;
}


// Source: openssl.c
// Lines 1373-1391
