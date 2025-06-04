static socklen_t get_peername(h2o_conn_t *_conn, struct sockaddr *sa)
{
    struct st_h2o_http1_conn_t *conn = (void *)_conn;
    return h2o_socket_getpeername(conn->sock, sa);
}


// Source: http1.c
// Lines 1163-1167
