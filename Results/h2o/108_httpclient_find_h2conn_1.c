static struct st_h2o_httpclient__h2_conn_t *find_h2conn(h2o_httpclient_connection_pool_t *pool, h2o_url_t *target)
{
    int should_check_target = h2o_socketpool_is_global(pool->socketpool);

    for (h2o_linklist_t *l = pool->http2.conns.next; l != &pool->http2.conns; l = l->next) {
        struct st_h2o_httpclient__h2_conn_t *conn = H2O_STRUCT_FROM_MEMBER(struct st_h2o_httpclient__h2_conn_t, link, l);
        if (should_check_target && !(conn->origin_url.scheme == target->scheme &&
                                     h2o_memis(conn->origin_url.authority.base, conn->origin_url.authority.len,
                                               target->authority.base, target->authority.len)))
            continue;
        if (conn->num_streams >= h2o_httpclient__h2_get_max_concurrent_streams(conn))
            continue;
        return conn;
    }

    return NULL;
}


// Source: httpclient.c
// Lines 196-212
