int quicly_streambuf_egress_shutdown(quicly_stream_t *stream)
{
    quicly_streambuf_t *sbuf = stream->data;
    quicly_sendstate_shutdown(&stream->sendstate, sbuf->egress.bytes_written);
    return quicly_stream_sync_sendbuf(stream, 1);
}


// Source: streambuf.c
// Lines 245-250
