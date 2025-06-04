int zipmapValidateIntegrity(unsigned char *zm, size_t size, int deep) {
#define OUT_OF_RANGE(p) ( \
        (p) < zm + 2 || \
        (p) > zm + size - 1)
    unsigned int l, s, e;

    /* check that we can actually read the header (or ZIPMAP_END). */
    if (size < 2)
        return 0;

    /* the last byte must be the terminator. */
    if (zm[size-1] != ZIPMAP_END)
        return 0;

    if (!deep)
        return 1;

    unsigned int count = 0;
    unsigned char *p = zm + 1; /* skip the count */
    while(*p != ZIPMAP_END) {
        /* read the field name length encoding type */
        s = zipmapGetEncodedLengthSize(p);
        /* make sure the entry length doesn't rech outside the edge of the zipmap */
        if (OUT_OF_RANGE(p+s))
            return 0;

        /* read the field name length */
        l = zipmapDecodeLength(p);
        p += s; /* skip the encoded field size */
        p += l; /* skip the field */

        /* make sure the entry doesn't rech outside the edge of the zipmap */
        if (OUT_OF_RANGE(p))
            return 0;

        /* read the value length encoding type */
        s = zipmapGetEncodedLengthSize(p);
        /* make sure the entry length doesn't rech outside the edge of the zipmap */
        if (OUT_OF_RANGE(p+s))
            return 0;

        /* read the value length */
        l = zipmapDecodeLength(p);
        p += s; /* skip the encoded value size*/
        e = *p++; /* skip the encoded free space (always encoded in one byte) */
        p += l+e; /* skip the value and free space */
        count++;

        /* make sure the entry doesn't rech outside the edge of the zipmap */
        if (OUT_OF_RANGE(p))
            return 0;
    }

    /* check that the zipmap is not empty. */
    if (count == 0) return 0;

    /* check that the count in the header is correct */
    if (zm[0] != ZIPMAP_BIGLEN && zm[0] != count)
        return 0;

    return 1;
#undef OUT_OF_RANGE
}


// Source: zipmap.c
// Lines 380-442
