static void egress_unistream_on_send_shift(quicly_stream_t *qs, size_t delta)
{
    struct st_h2o_http3_egress_unistream_t *stream = qs->data;
    h2o_buffer_consume(&stream->sendbuf, delta);
}


// Source: common.c
// Lines 365-369
