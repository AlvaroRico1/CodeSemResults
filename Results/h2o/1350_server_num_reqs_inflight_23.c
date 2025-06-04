static uint32_t num_reqs_inflight(h2o_conn_t *_conn)
{
    struct st_h2o_http3_server_conn_t *conn = (void *)_conn;
    return quicly_num_streams_by_group(conn->h3.super.quic, 0, 0);
}


// Source: server.c
// Lines 505-509
