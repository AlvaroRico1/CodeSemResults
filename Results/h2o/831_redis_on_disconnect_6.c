static void on_disconnect(const redisAsyncContext *redis, int status)
{
    h2o_redis_client_t *client = (h2o_redis_client_t *)redis->data;
    if (client == NULL)
        return;

    close_and_detach_connection(client, get_error(redis));
}


// Source: redis.c
// Lines 111-118
