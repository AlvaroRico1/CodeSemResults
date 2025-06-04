int h2o_http3_decode_goaway_frame(h2o_http3_goaway_frame_t *frame, const uint8_t *payload, size_t len, const char **err_desc)
{
    const uint8_t *src = payload, *end = src + len;

    if ((frame->stream_or_push_id = quicly_decodev(&src, end)) == UINT64_MAX)
        goto Fail;

    if (src != end) {
        /* there was an extra byte(s) after a valid QUIC variable-length integer */
        goto Fail;
    }

    return 0;

Fail:
    *err_desc = "Invalid GOAWAY frame";
    return H2O_HTTP3_ERROR_FRAME;
}


// Source: frame.c
// Lines 82-99
