static ptls_t *get_ptls(h2o_conn_t *_conn)
{
    struct st_h2o_http1_conn_t *conn = (void *)_conn;
    assert(conn->sock != NULL && "it never becomes NULL, right?");
    return h2o_socket_get_ptls(conn->sock);
}


// Source: http1.c
// Lines 1169-1174
