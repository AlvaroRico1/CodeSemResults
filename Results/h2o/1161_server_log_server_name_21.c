static h2o_iovec_t log_server_name(h2o_req_t *req)
{
    struct st_h2o_http3_server_conn_t *conn = (struct st_h2o_http3_server_conn_t *)req->conn;
    ptls_t *tls = quicly_get_tls(conn->h3.super.quic);
    const char *server_name = ptls_get_server_name(tls);
    return server_name != NULL ? h2o_iovec_init(server_name, strlen(server_name)) : h2o_iovec_init(NULL, 0);
}


// Source: server.c
// Lines 579-585
