void quicly_streambuf_destroy(quicly_stream_t *stream, int err)
{
    quicly_streambuf_t *sbuf = stream->data;

    quicly_sendbuf_dispose(&sbuf->egress);
    ptls_buffer_dispose(&sbuf->ingress);
    free(sbuf);
    stream->data = NULL;
}


// Source: streambuf.c
// Lines 229-237
