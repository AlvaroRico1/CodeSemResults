static void egress_unistream_on_destroy(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3_egress_unistream_t *stream = qs->data;
    h2o_buffer_dispose(&stream->sendbuf);
    free(stream);
}


// Source: common.c
// Lines 358-363
