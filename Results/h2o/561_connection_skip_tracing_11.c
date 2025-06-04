static int skip_tracing(h2o_conn_t *_conn)
{
    struct st_h2o_http2_conn_t *conn = (void *)_conn;
    assert(conn->sock != NULL && "it never becomes NULL, right?");
    return h2o_socket_skip_tracing(conn->sock);
}


// Source: connection.c
// Lines 1567-1572
