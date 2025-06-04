static void destroy_connection_on_transport_close(h2o_quic_conn_t *_conn)
{
    struct st_h2o_httpclient__h3_conn_t *conn = (void *)_conn;

    /* When a connection gets closed while request is inflight, the most probable cause is some error in the transport (or at the
     * application protocol layer). But as we do not know the exact cause, we use a generic error here. */
    destroy_connection(conn, h2o_httpclient_error_io);
}


// Source: http3client.c
// Lines 238-245
