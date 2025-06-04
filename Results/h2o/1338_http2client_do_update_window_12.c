static void do_update_window(h2o_httpclient_t *_client)
{
    struct st_h2o_http2client_stream_t *stream = (void *)_client;
    size_t max = get_max_buffer_size(stream->super.ctx);
    size_t bufsize = stream->input.body->size;
    assert(bufsize <= max);
    enqueue_window_update(stream->conn, stream->stream_id, &stream->input.window, max - bufsize);
}


// Source: http2client.c
// Lines 1285-1292
