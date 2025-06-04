static h2o_iovec_t log_delivery_rate(h2o_req_t *req)
{
    struct st_h2o_http3_server_conn_t *conn = (struct st_h2o_http3_server_conn_t *)req->conn;
    quicly_rate_t rate;

    if (quicly_get_delivery_rate(conn->h3.super.quic, &rate) == 0 && rate.latest != 0) {
        char *buf = h2o_mem_alloc_pool(&req->pool, char, sizeof(H2O_UINT64_LONGEST_STR));
        size_t len = sprintf(buf, "%" PRIu64, rate.latest);
        return h2o_iovec_init(buf, len);
    }

    return h2o_iovec_init(NULL, 0);
}


// Source: server.c
// Lines 527-539
