static quicly_stream_t *open_stream(quicly_conn_t *conn, uint64_t stream_id, uint32_t initial_max_stream_data_local,
                                    uint64_t initial_max_stream_data_remote)
{
    quicly_stream_t *stream;

    if ((stream = malloc(sizeof(*stream))) == NULL)
        return NULL;
    stream->conn = conn;
    stream->stream_id = stream_id;
    stream->callbacks = NULL;
    stream->data = NULL;

    int r;
    khiter_t iter = kh_put(quicly_stream_t, conn->streams, stream_id, &r);
    assert(iter != kh_end(conn->streams));
    kh_val(conn->streams, iter) = stream;

    init_stream_properties(stream, initial_max_stream_data_local, initial_max_stream_data_remote);

    return stream;
}


// Source: quicly.c
// Lines 1065-1085
