size_t h2o_qpack_decoder_send_state_sync(h2o_qpack_decoder_t *qpack, uint8_t *outbuf)
{
    if (qpack->insert_count == 0)
        return 0;

    uint8_t *dst = outbuf;
    *dst = 0;
    dst = h2o_hpack_encode_int(dst, qpack->insert_count, 6);
    qpack->insert_count = 0;

    return dst - outbuf;
}


// Source: qpack.c
// Lines 520-531
