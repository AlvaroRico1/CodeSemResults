int ptls_buffer_push_asn1_ubigint(ptls_buffer_t *buf, const void *bignum, size_t size)
{
    const uint8_t *p = bignum, *const end = p + size;
    int ret;

    /* skip zeroes */
    for (; end - p >= 1; ++p)
        if (*p != 0)
            break;

    /* emit */
    ptls_buffer_push(buf, 2);
    ptls_buffer_push_asn1_block(buf, {
        if (*p >= 0x80)
            ptls_buffer_push(buf, 0);
        if (p != end) {
            ptls_buffer_pushv(buf, p, end - p);
        } else {
            ptls_buffer_pushv(buf, "", 1);
        }
    });
    ret = 0;

Exit:
    return ret;
}


// Source: picotls.c
// Lines 580-605
