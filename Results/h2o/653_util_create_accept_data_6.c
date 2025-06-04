static struct st_h2o_accept_data_t *create_accept_data(h2o_accept_ctx_t *ctx, h2o_socket_t *sock, struct timeval connected_at,
                                                       h2o_timer_cb timeout_cb, size_t sz)
{
    struct st_h2o_accept_data_t *data = h2o_mem_alloc(sz);
    data->ctx = ctx;
    data->sock = sock;
    h2o_timer_init(&data->timeout, timeout_cb);
    h2o_timer_link(ctx->ctx->loop, ctx->ctx->globalconf->handshake_timeout, &data->timeout);
    data->connected_at = connected_at;
    return data;
}


// Source: util.c
// Lines 67-77
