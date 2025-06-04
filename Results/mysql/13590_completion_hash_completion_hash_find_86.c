static Bucket *completion_hash_find(HashTable *ht, const char *arKey,
                                    uint nKeyLength) {
  uint h, nIndex;
  Bucket *p;

  h = ht->pHashFunction(arKey, nKeyLength);
  nIndex = h % ht->nTableSize;

  p = ht->arBuckets[nIndex];
  while (p) {
    if ((p->h == h) && (p->nKeyLength == nKeyLength)) {
      if (!memcmp(p->arKey, arKey, nKeyLength)) {
        return p;
      }
    }
    p = p->pNext;
  }
  return (Bucket *)nullptr;
}


// Source: completion_hash.cc
// Lines 111-129
