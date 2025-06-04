static h2o_iovec_t log_negotiated_protocol(h2o_req_t *req)
{
    struct st_h2o_http3_server_conn_t *conn = (struct st_h2o_http3_server_conn_t *)req->conn;
    ptls_t *tls = quicly_get_tls(conn->h3.super.quic);
    const char *proto = ptls_get_negotiated_protocol(tls);
    return proto != NULL ? h2o_iovec_init(proto, strlen(proto)) : h2o_iovec_init(NULL, 0);
}


// Source: server.c
// Lines 587-593
