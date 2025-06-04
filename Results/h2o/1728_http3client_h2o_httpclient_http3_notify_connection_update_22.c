void h2o_httpclient_http3_notify_connection_update(h2o_quic_ctx_t *ctx, h2o_quic_conn_t *_conn)
{
    struct st_h2o_httpclient__h3_conn_t *conn = (void *)_conn;

    if (h2o_timer_is_linked(&conn->timeout) && conn->timeout.cb == on_connect_timeout) {
        /* TODO check connection state? */
        h2o_timer_unlink(&conn->timeout);
    }
}


// Source: http3client.c
// Lines 882-890
