static void disconnect(h2o_redis_client_t *client, const char *errstr)
{
    assert(client->state != H2O_REDIS_CONNECTION_STATE_CLOSED);
    assert(client->_redis != NULL);

    redisAsyncContext *redis = client->_redis;
    struct st_redis_socket_data_t *data = redis->ev.data;
    data->errstr = errstr;
    close_and_detach_connection(client, errstr);
    redisAsyncFree(redis); /* immediately call all callbacks of pending commands with nil replies */
}


// Source: redis.c
// Lines 58-68
