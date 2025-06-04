static int foreach_request_per_conn(void *_ctx, quicly_stream_t *qs)
{
    struct foreach_request_ctx *ctx = _ctx;

    /* skip if the stream is not a request stream (TODO handle push?) */
    if (!(quicly_stream_is_client_initiated(qs->stream_id) && !quicly_stream_is_unidirectional(qs->stream_id)))
        return 0;

    struct st_h2o_http3_server_stream_t *stream = qs->data;
    assert(stream->quic == qs);

    if (stream->state == H2O_HTTP3_SERVER_STREAM_STATE_CLOSE_WAIT)
        return 0;
    return ctx->cb(&stream->req, ctx->cbdata);
}


// Source: server.c
// Lines 1980-1994
