int getBitOffsetFromArgument(client *c, robj *o, uint64_t *offset, int hash, int bits) {
    long long loffset;
    char *err = "bit offset is not an integer or out of range";
    char *p = o->ptr;
    size_t plen = sdslen(p);
    int usehash = 0;

    /* Handle #<offset> form. */
    if (p[0] == '#' && hash && bits > 0) usehash = 1;

    if (string2ll(p+usehash,plen-usehash,&loffset) == 0) {
        addReplyError(c,err);
        return C_ERR;
    }

    /* Adjust the offset by 'bits' for #<offset> form. */
    if (usehash) loffset *= bits;

    /* Limit offset to server.proto_max_bulk_len (512MB in bytes by default) */
    if ((loffset < 0) || (loffset >> 3) >= server.proto_max_bulk_len)
    {
        addReplyError(c,err);
        return C_ERR;
    }

    *offset = loffset;
    return C_OK;
}


// Source: bitops.c
// Lines 413-440
