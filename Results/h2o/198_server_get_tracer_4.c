static quicly_tracer_t *get_tracer(h2o_conn_t *_conn)
{
    struct st_h2o_http3_server_conn_t *conn = (void *)_conn;
    return quicly_get_tracer(conn->h3.super.quic);
}


// Source: server.c
// Lines 511-515
