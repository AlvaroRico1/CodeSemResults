static void memcached_resumption_get(h2o_socket_t *sock, h2o_iovec_t session_id)
{
    struct st_h2o_memcached_resumption_accept_data_t *data = sock->data;

    data->get_req = h2o_memcached_get(async_resumption_context.memcached.ctx, data->super.ctx->libmemcached_receiver, session_id,
                                      memcached_resumption_on_get, data, H2O_MEMCACHED_ENCODE_KEY | H2O_MEMCACHED_ENCODE_VALUE);
}


// Source: util.c
// Lines 145-151
