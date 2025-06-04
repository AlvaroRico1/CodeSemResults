static int do_write_req(h2o_httpclient_t *_client, h2o_iovec_t chunk, int is_end_stream)
{
    struct st_h2o_http2client_stream_t *stream = (void *)_client;
    assert(stream->streaming.proceed_req != NULL);
    assert(!stream->streaming.inflight);

    stream->streaming.inflight = 1;
    if (is_end_stream)
        stream->streaming.done = 1;

    if (stream->output.buf == NULL) {
        h2o_buffer_init(&stream->output.buf, &h2o_socket_buffer_prototype);
    }

    if (chunk.len != 0) {
        h2o_buffer_append(&stream->output.buf, chunk.base, chunk.len);
    }

    if (!h2o_linklist_is_linked(&stream->output.sending_link)) {
        h2o_linklist_insert(&stream->conn->output.sending_streams, &stream->output.sending_link);
        request_write(stream->conn);
    }

    return 0;
}


// Source: http2client.c
// Lines 1294-1318
