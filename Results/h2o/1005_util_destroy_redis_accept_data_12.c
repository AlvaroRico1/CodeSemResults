static void destroy_redis_accept_data(struct st_h2o_accept_data_t *_accept_data)
{
    struct st_h2o_redis_resumption_accept_data_t *accept_data = (struct st_h2o_redis_resumption_accept_data_t *)_accept_data;
    assert(accept_data->get_command == NULL);
    destroy_accept_data(&accept_data->super);
}


// Source: util.c
// Lines 115-120
