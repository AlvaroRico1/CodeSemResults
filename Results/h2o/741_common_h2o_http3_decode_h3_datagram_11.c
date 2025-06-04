uint64_t h2o_http3_decode_h3_datagram(h2o_iovec_t *payload, const void *_src, size_t len)
{
    const uint8_t *src = _src, *end = src + len;
    uint64_t flow_id;

    if ((flow_id = ptls_decode_quicint(&src, end)) != UINT64_MAX)
        *payload = h2o_iovec_init(src, end - src);
    return flow_id;
}


// Source: common.c
// Lines 1348-1356
