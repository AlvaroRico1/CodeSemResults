int completion_hash_update(HashTable *ht, const char *arKey, uint nKeyLength,
                           const char *str) {
  uint h, nIndex;

  Bucket *p;

  h = ht->pHashFunction(arKey, nKeyLength);
  nIndex = h % ht->nTableSize;

  if (nKeyLength <= 0) {
    return FAILURE;
  }
  p = ht->arBuckets[nIndex];
  while (p) {
    if ((p->h == h) && (p->nKeyLength == nKeyLength)) {
      if (!memcmp(p->arKey, arKey, nKeyLength)) {
        entry *n;

        if (!(n = (entry *)ht->mem_root.Alloc(sizeof(entry)))) return FAILURE;
        n->pNext = p->pData;
        n->str = str;
        p->pData = n;
        p->count++;

        return SUCCESS;
      }


// Source: completion_hash.cc
// Lines 63-88
