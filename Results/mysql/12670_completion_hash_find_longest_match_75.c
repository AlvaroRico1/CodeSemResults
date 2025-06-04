Bucket *find_longest_match(HashTable *ht, char *str, uint length,
                           uint *res_length) {
  Bucket *b, *return_b;
  const char *s;
  uint count;
  uint lm;

  b = completion_hash_find(ht, str, length);
  if (!b) {
    *res_length = 0;
    return (Bucket *)nullptr;
  }

  count = b->count;
  lm = length;
  s = b->pData->str;

  return_b = b;
  while (s[lm] != 0 && (b = completion_hash_find(ht, s, lm + 1))) {
    if (b->count < count) {
      *res_length = lm;
      return return_b;
    }
    return_b = b;
    lm++;
  }
  *res_length = lm;
  return return_b;
}


// Source: completion_hash.cc
// Lines 164-192
