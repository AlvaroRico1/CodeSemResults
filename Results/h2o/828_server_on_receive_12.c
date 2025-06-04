static void on_receive(quicly_stream_t *qs, size_t off, const void *input, size_t len)
{
    struct st_h2o_http3_server_stream_t *stream = qs->data;

    /* save received data (FIXME avoid copying if possible; see hqclient.c) */
    h2o_http3_update_recvbuf(&stream->recvbuf.buf, off, input, len);

    if (stream->read_blocked)
        return;

    /* handle input (FIXME propage err_desc) */
    handle_buffered_input(stream, 0);
}


// Source: server.c
// Lines 916-928
