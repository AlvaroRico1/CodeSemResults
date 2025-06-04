void quicklistAppendZiplist(quicklist *quicklist, unsigned char *zl) {
    quicklistNode *node = quicklistCreateNode();

    node->zl = zl;
    node->count = ziplistLen(node->zl);
    node->sz = ziplistBlobLen(zl);

    _quicklistInsertNodeAfter(quicklist, quicklist->tail, node);
    quicklist->count += node->count;
}


// Source: quicklist.c
// Lines 539-548
