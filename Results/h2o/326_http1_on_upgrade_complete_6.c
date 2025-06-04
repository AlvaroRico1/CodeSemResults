static void on_upgrade_complete(h2o_socket_t *socket, const char *err)
{
    struct st_h2o_http1_conn_t *conn = socket->data;
    h2o_http1_upgrade_cb cb = conn->upgrade.cb;
    void *data = conn->upgrade.data;
    h2o_socket_t *sock = NULL;
    size_t headers_size = 0;

    /* destruct the connection (after detaching the socket) */
    if (err == 0) {
        sock = conn->sock;
        headers_size = conn->_unconsumed_request_size;
        close_connection(conn, 0);
    } else {
        close_connection(conn, 1);
    }

    cb(data, sock, headers_size);
}


// Source: http1.c
// Lines 856-874
