// Source: curl/lib/hash.c
// Lines 85-88
mk_hash_element(const void *key, size_t key_len, const void *p)
{
  /* allocate the struct plus memory after it to store the key */
  struct Curl_hash_element *he = malloc(sizeof(struct Curl_hash_element) +
                                        key_len);
  if(he) {
    /* copy the key */
    memcpy(he->key, key, key_len);
    he->key_len = key_len;
    he->ptr = (void *) p;
  }
  return he;
}
