unsigned int ziplistLen(unsigned char *zl) {
    unsigned int len = 0;
    if (intrev16ifbe(ZIPLIST_LENGTH(zl)) < UINT16_MAX) {
        len = intrev16ifbe(ZIPLIST_LENGTH(zl));
    } else {
        unsigned char *p = zl+ZIPLIST_HEADER_SIZE;
        size_t zlbytes = intrev32ifbe(ZIPLIST_BYTES(zl));
        while (*p != ZIP_END) {
            p += zipRawEntryLengthSafe(zl, zlbytes, p);
            len++;
        }

        /* Re-store length if small enough */
        if (len < UINT16_MAX) ZIPLIST_LENGTH(zl) = intrev16ifbe(len);
    }


// Source: ziplist.c
// Lines 1408-1422
