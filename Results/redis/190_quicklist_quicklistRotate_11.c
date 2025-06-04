void quicklistRotate(quicklist *quicklist) {
    if (quicklist->count <= 1)
        return;

    /* First, get the tail entry */
    unsigned char *p = ziplistIndex(quicklist->tail->zl, -1);
    unsigned char *value, *tmp;
    long long longval;
    unsigned int sz;
    char longstr[32] = {0};
    ziplistGet(p, &tmp, &sz, &longval);

    /* If value found is NULL, then ziplistGet populated longval instead */
    if (!tmp) {
        /* Write the longval as a string so we can re-add it */
        sz = ll2string(longstr, sizeof(longstr), longval);
        value = (unsigned char *)longstr;
    } else if (quicklist->len == 1) {
        /* Copy buffer since there could be a memory overlap when move
         * entity from tail to head in the same ziplist. */
        value = zmalloc(sz);
        memcpy(value, tmp, sz);
    } else {
        value = tmp;
    }


// Source: quicklist.c
// Lines 1307-1331
