static void on_read(h2o_socket_t *sock, const char *err)
{
    struct st_h2o_http2client_conn_t *conn = sock->data;

    h2o_timer_unlink(&conn->io_timeout);

    if (err != NULL) {
        call_stream_callbacks_with_error(conn, h2o_httpclient_error_io);
        close_connection(conn);
        return;
    }

    if (parse_input(conn) != 0)
        return;

    /* write immediately, if pending write exists */
    if (h2o_timer_is_linked(&conn->output.defer_timeout)) {
        h2o_timer_unlink(&conn->output.defer_timeout);
        do_emit_writereq(conn);
    }

    if (!h2o_timer_is_linked(&conn->io_timeout))
        h2o_timer_link(conn->super.ctx->loop, conn->super.ctx->io_timeout, &conn->io_timeout);
}


// Source: http2client.c
// Lines 988-1011
