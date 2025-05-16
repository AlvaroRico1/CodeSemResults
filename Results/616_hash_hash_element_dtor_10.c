hash_element_dtor(void *user, void *element)
{
  struct Curl_hash *h = (struct Curl_hash *) user;
  struct Curl_hash_element *e = (struct Curl_hash_element *) element;

  if(e->ptr) {
    h->dtor(e->ptr);
    e->ptr = NULL;
  }

  e->key_len = 0;

  free(e);
}


// Source: hash.c
// Lines 35-48
