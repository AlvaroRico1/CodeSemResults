static void on_notify_write(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_http2client_conn_t *conn = sock->data;

    if (err != NULL) {
        call_stream_callbacks_with_error(conn, h2o_httpclient_error_io);
        close_connection_now(conn);
        return;
    }
    do_emit_writereq(conn);
    close_connection_if_necessary(conn);
}


// Source: http2client.c
// Lines 1062-1073
