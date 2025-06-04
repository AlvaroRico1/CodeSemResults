static h2o_httpclient_t *create_client(h2o_httpclient_t **_client, h2o_mem_pool_t *pool, void *data, h2o_httpclient_ctx_t *ctx,
                                       h2o_httpclient_connection_pool_t *connpool, const char *upgrade_to,
                                       h2o_httpclient_connect_cb on_connect)
{
#define SZ_MAX(x, y) ((x) > (y) ? (x) : (y))
    size_t sz = SZ_MAX(h2o_httpclient__h1_size, h2o_httpclient__h2_size);
#undef SZ_MAX
    h2o_httpclient_t *client = h2o_mem_alloc(sz);
    memset(client, 0, sz);
    client->pool = pool;
    client->ctx = ctx;
    client->data = data;
    client->upgrade_to = upgrade_to;
    client->connpool = connpool;
    client->cancel = do_cancel;
    client->_cb.on_connect = on_connect;
    client->_timeout.cb = on_connect_timeout;
    client->timings.start_at = h2o_gettimeofday(ctx->loop);

    if (_client != NULL)
        *_client = client;

    return client;
}


// Source: httpclient.c
// Lines 85-108
