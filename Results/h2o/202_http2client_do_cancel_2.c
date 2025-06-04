static void do_cancel(h2o_httpclient_t *_client)
{
    struct st_h2o_http2client_stream_t *stream = (void *)_client;
    stream_send_error(stream->conn, stream->stream_id, H2O_HTTP2_ERROR_CANCEL);
    close_stream(stream);
}


// Source: http2client.c
// Lines 1272-1277
