void quicly_streambuf_egress_emit(quicly_stream_t *stream, size_t off, void *dst, size_t *len, int *wrote_all)
{
    quicly_streambuf_t *sbuf = stream->data;
    quicly_sendbuf_emit(stream, &sbuf->egress, off, dst, len, wrote_all);
}


// Source: streambuf.c
// Lines 239-243
