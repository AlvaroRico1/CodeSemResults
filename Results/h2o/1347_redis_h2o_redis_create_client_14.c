h2o_redis_client_t *h2o_redis_create_client(h2o_loop_t *loop, size_t sz)
{
    h2o_redis_client_t *client = h2o_mem_alloc(sz);
    memset(client, 0, sz);

    client->loop = loop;
    client->state = H2O_REDIS_CONNECTION_STATE_CLOSED;
    h2o_timer_init(&client->_timeout_entry, on_connect_timeout);

    return client;
}


// Source: redis.c
// Lines 129-139
