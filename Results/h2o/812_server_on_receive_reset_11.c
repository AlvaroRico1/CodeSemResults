static void on_receive_reset(quicly_stream_t *qs, int err)
{
    struct st_h2o_http3_server_stream_t *stream = qs->data;

    shutdown_stream(stream, H2O_HTTP3_ERROR_NONE /* ignored */,
                    stream->state == H2O_HTTP3_SERVER_STREAM_STATE_RECV_HEADERS ? H2O_HTTP3_ERROR_REQUEST_REJECTED
                                                                                : H2O_HTTP3_ERROR_REQUEST_CANCELLED,
                    0);
}


// Source: server.c
// Lines 930-938
