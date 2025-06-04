static void on_connect(const redisAsyncContext *redis, int status)
{
    h2o_redis_client_t *client = (h2o_redis_client_t *)redis->data;
    if (client == NULL)
        return;

    if (status != REDIS_OK) {
        close_and_detach_connection(client, h2o_redis_error_connection);
        return;
    }
    h2o_timer_unlink(&client->_timeout_entry);

    client->state = H2O_REDIS_CONNECTION_STATE_CONNECTED;
    if (client->on_connect != NULL)
        client->on_connect();
}


// Source: redis.c
// Lines 94-109
