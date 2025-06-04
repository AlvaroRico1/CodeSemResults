int quicly_streambuf_create(quicly_stream_t *stream, size_t sz)
{
    quicly_streambuf_t *sbuf;

    assert(sz >= sizeof(*sbuf));
    assert(stream->data == NULL);

    if ((sbuf = malloc(sz)) == NULL)
        return PTLS_ERROR_NO_MEMORY;
    quicly_sendbuf_init(&sbuf->egress);
    ptls_buffer_init(&sbuf->ingress, "", 0);
    if (sz != sizeof(*sbuf))
        memset((char *)sbuf + sizeof(*sbuf), 0, sz - sizeof(*sbuf));

    stream->data = sbuf;
    return 0;
}


// Source: streambuf.c
// Lines 211-227
