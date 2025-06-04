static void on_command(redisAsyncContext *redis, void *_reply, void *privdata)
{
    redisReply *reply = (redisReply *)_reply;
    h2o_redis_command_t *command = (h2o_redis_command_t *)privdata;
    const char *errstr = ((struct st_redis_socket_data_t *)redis->ev.data)->errstr;
    if (errstr == NULL)
        errstr = get_error(redis);
    handle_reply(command, reply, errstr);
}


// Source: redis.c
// Lines 206-214
