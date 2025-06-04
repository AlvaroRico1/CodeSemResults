static void destroy_memcached_accept_data(struct st_h2o_accept_data_t *_accept_data)
{
    struct st_h2o_memcached_resumption_accept_data_t *accept_data =
        (struct st_h2o_memcached_resumption_accept_data_t *)_accept_data;
    assert(accept_data->get_req == NULL);
    destroy_accept_data(&accept_data->super);
}


// Source: util.c
// Lines 122-128
