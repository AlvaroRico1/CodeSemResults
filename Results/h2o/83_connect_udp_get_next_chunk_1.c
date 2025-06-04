static h2o_iovec_t udp_get_next_chunk(const char *start, size_t len, size_t *to_consume, int *skip)
{
    const uint8_t *bytes = (const uint8_t *)start;
    const uint8_t *end = bytes + len;
    uint64_t chunk_type, chunk_length;

    chunk_type = ptls_decode_quicint(&bytes, end);
    if (chunk_type == UINT64_MAX)
        return h2o_iovec_init(NULL, 0);
    chunk_length = ptls_decode_quicint(&bytes, end);
    if (chunk_length == UINT64_MAX)
        return h2o_iovec_init(NULL, 0);

    /* chunk is incomplete */
    if (end - bytes < chunk_length)
        return h2o_iovec_init(NULL, 0);

    /*
     * https://tools.ietf.org/html/draft-ietf-masque-connect-udp-03#section-6
     * CONNECT-UDP Stream Chunks can be used to convey UDP payloads, by
     * using a CONNECT-UDP Stream Chunk Type of UDP_PACKET (value 0x00).
     */
    *skip = chunk_type != 0;
    *to_consume = (bytes + chunk_length) - (const uint8_t *)start;

    return h2o_iovec_init(bytes, chunk_length);
}


// Source: connect.c
// Lines 607-633
