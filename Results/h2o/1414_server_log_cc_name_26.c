static h2o_iovec_t log_cc_name(h2o_req_t *req)
{
    struct st_h2o_http3_server_conn_t *conn = (struct st_h2o_http3_server_conn_t *)req->conn;
    quicly_stats_t stats;

    if (quicly_get_stats(conn->h3.super.quic, &stats) == 0)
        return h2o_iovec_init(stats.cc.type->name, strlen(stats.cc.type->name));
    return h2o_iovec_init(NULL, 0);
}


// Source: server.c
// Lines 517-525
