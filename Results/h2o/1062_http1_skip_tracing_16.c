static int skip_tracing(h2o_conn_t *_conn)
{
    struct st_h2o_http1_conn_t *conn = (void *)_conn;
    return h2o_socket_skip_tracing(conn->sock);
}


// Source: http1.c
// Lines 1176-1180
