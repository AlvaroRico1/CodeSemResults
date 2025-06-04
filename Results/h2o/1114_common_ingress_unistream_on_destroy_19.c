static void ingress_unistream_on_destroy(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3_ingress_unistream_t *stream = qs->data;
    h2o_buffer_dispose(&stream->recvbuf);
    free(stream);
}


// Source: common.c
// Lines 203-208
