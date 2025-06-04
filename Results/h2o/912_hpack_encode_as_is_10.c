static size_t encode_as_is(uint8_t *dst, const char *s, size_t len)
{
    uint8_t *start = dst;
    *dst = '\0';
    dst = h2o_hpack_encode_int(dst, len, 7);
    memcpy(dst, s, len);
    dst += len;
    return dst - start;
}


// Source: hpack.c
// Lines 714-722
