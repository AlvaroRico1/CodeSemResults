static size_t decode_quic_forwarded_header(quicly_address_t *destaddr, quicly_address_t *srcaddr, uint8_t *ttl, h2o_iovec_t octets)
{
    const uint8_t *src = (uint8_t *)octets.base, *end = src + octets.len;

    if (end - src < 6)
        goto NotForwarded;
    if (*src++ != 0x80)
        goto NotForwarded;
    if (quicly_decode32(&src) != H2O_QUIC_FORWARDED_VERSION)
        goto NotForwarded;
    if (!decode_quic_address(destaddr, &src, end))
        goto NotForwarded;
    if (!decode_quic_address(srcaddr, &src, end))
        goto NotForwarded;
    if (end - src < 1)
        goto NotForwarded;
    *ttl = *src++;

    return src - (const uint8_t *)octets.base;
NotForwarded:
    return SIZE_MAX;
}


// Source: main.c
// Lines 2716-2737
