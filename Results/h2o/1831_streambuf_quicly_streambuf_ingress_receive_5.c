int quicly_streambuf_ingress_receive(quicly_stream_t *stream, size_t off, const void *src, size_t len)
{
    quicly_streambuf_t *sbuf = stream->data;
    return quicly_recvbuf_receive(stream, &sbuf->ingress, off, src, len);
}


// Source: streambuf.c
// Lines 252-256
