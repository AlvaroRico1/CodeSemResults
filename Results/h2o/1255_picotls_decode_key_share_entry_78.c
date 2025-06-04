static int decode_key_share_entry(uint16_t *group, ptls_iovec_t *key_exchange, const uint8_t **src, const uint8_t *const end)
{
    int ret;

    if ((ret = ptls_decode16(group, src, end)) != 0)
        goto Exit;
    ptls_decode_open_block(*src, end, 2, {
        *key_exchange = ptls_iovec_init(*src, end - *src);
        *src = end;
    });

Exit:
    return ret;
}


// Source: picotls.c
// Lines 1621-1634
