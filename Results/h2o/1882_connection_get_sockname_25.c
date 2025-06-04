static socklen_t get_sockname(h2o_conn_t *_conn, struct sockaddr *sa)
{
    h2o_http2_conn_t *conn = (void *)_conn;
    return h2o_socket_getsockname(conn->sock, sa);
}


// Source: connection.c
// Lines 1548-1552
