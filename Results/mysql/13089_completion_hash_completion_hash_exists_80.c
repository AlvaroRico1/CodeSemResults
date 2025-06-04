int completion_hash_exists(HashTable *ht, char *arKey, uint nKeyLength) {
  uint h, nIndex;
  Bucket *p;

  h = ht->pHashFunction(arKey, nKeyLength);
  nIndex = h % ht->nTableSize;

  p = ht->arBuckets[nIndex];
  while (p) {
    if ((p->h == h) && (p->nKeyLength == nKeyLength)) {
      if (!strcmp(p->arKey, arKey)) {
        return 1;
      }
    }
    p = p->pNext;
  }
  return 0;
}


// Source: completion_hash.cc
// Lines 131-148
