void reqread_on_read(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_http1_conn_t *conn = sock->data;

    if (err != NULL) {
        close_connection(conn, 1);
        return;
    }

    set_req_io_timeout(conn, conn->super.ctx->globalconf->http1.req_io_timeout, req_io_on_timeout);
    if (conn->_req_entity_reader == NULL)
        handle_incoming_request(conn);
    else
        conn->_req_entity_reader->handle_incoming_entity(conn);
}


// Source: http1.c
// Lines 747-761
