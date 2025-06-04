void add_word(HashTable *ht, const char *str) {
  int i;
  const char *pos = str;
  for (i = 1; *pos; i++, pos++) completion_hash_update(ht, str, i, str);
}


// Source: completion_hash.cc
// Lines 204-208
