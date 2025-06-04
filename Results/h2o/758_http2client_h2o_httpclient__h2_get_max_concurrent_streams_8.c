uint32_t h2o_httpclient__h2_get_max_concurrent_streams(h2o_httpclient__h2_conn_t *_conn)
{
    struct st_h2o_http2client_conn_t *conn = (void *)_conn;
    return conn->peer_settings.max_concurrent_streams < conn->super.ctx->http2.max_concurrent_streams
               ? conn->peer_settings.max_concurrent_streams
               : conn->super.ctx->http2.max_concurrent_streams;
}


// Source: http2client.c
// Lines 157-163
