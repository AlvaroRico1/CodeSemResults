void h2o_redis_connect(h2o_redis_client_t *client, const char *host, uint16_t port)
{
    if (client->state != H2O_REDIS_CONNECTION_STATE_CLOSED) {
        return;
    }

    redisAsyncContext *redis = redisAsyncConnect(host, port);
    if (redis == NULL) {
        h2o_fatal("no memory");
    }

    client->_redis = redis;
    client->_redis->data = client;
    client->state = H2O_REDIS_CONNECTION_STATE_CONNECTING;

    attach_loop(redis, client->loop);
    redisAsyncSetConnectCallback(redis, on_connect);
    redisAsyncSetDisconnectCallback(redis, on_disconnect);

    if (redis->err != REDIS_OK) {
        /* some connection failures can be detected at this time */
        disconnect(client, h2o_redis_error_connection);
        return;
    }

    if (client->connect_timeout != 0)
        h2o_timer_link(client->loop, client->connect_timeout, &client->_timeout_entry);
}


// Source: redis.c
// Lines 141-168
