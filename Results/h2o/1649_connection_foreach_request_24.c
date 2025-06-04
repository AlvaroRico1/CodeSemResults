static int foreach_request(h2o_context_t *ctx, int (*cb)(h2o_req_t *req, void *cbdata), void *cbdata)
{
    h2o_linklist_t *node;

    for (node = ctx->http2._conns.next; node != &ctx->http2._conns; node = node->next) {
        h2o_http2_conn_t *conn = H2O_STRUCT_FROM_MEMBER(h2o_http2_conn_t, _conns, node);
        h2o_http2_stream_t *stream;
        kh_foreach_value(conn->streams, stream, {
            int ret = cb(&stream->req, cbdata);
            if (ret != 0)
                return ret;
        });
    }
    return 0;
}


// Source: connection.c
// Lines 1847-1861
