int h2o_http3_decode_priority_update_frame(h2o_http3_priority_update_frame_t *frame, const uint8_t *payload, size_t len,
                                           const char **err_desc)
{
    const uint8_t *src = payload, *end = src + len;

    if (src == end)
        return H2O_HTTP3_ERROR_FRAME;
    frame->element_is_push = (*src++ & 0x80) != 0;
    if ((frame->element = quicly_decodev(&src, end)) == UINT64_MAX) {
        *err_desc = "invalid PRIORITY frame";
        return H2O_HTTP3_ERROR_FRAME;
    }
    if (frame->element_is_push) {
        if (!(!quicly_stream_is_client_initiated(frame->element) && quicly_stream_is_unidirectional(frame->element)))
            return H2O_HTTP3_ERROR_FRAME;
    } else {
        if (!(quicly_stream_is_client_initiated(frame->element) && !quicly_stream_is_unidirectional(frame->element)))
            return H2O_HTTP3_ERROR_FRAME;
    }
    frame->priority = h2o_absprio_default;
    h2o_absprio_parse_priority((const char *)src, end - src, &frame->priority);

    return 0;
}


// Source: frame.c
// Lines 41-64
