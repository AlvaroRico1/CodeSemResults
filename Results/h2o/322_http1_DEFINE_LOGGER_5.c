DEFINE_LOGGER(tcp_delivery_rate)
DEFINE_LOGGER(ssl_protocol_version)
DEFINE_LOGGER(ssl_session_reused)
DEFINE_LOGGER(ssl_cipher)
DEFINE_LOGGER(ssl_cipher_bits)
DEFINE_LOGGER(ssl_session_id)
DEFINE_LOGGER(ssl_server_name)
DEFINE_LOGGER(ssl_negotiated_protocol)

#undef DEFINE_LOGGER

static h2o_iovec_t log_request_index(h2o_req_t *req)
{
    struct st_h2o_http1_conn_t *conn = (void *)req->conn;
    char *s = h2o_mem_alloc_pool(&req->pool, char, sizeof(H2O_UINT64_LONGEST_STR));
    size_t len = sprintf(s, "%" PRIu64, conn->_req_index);
    return h2o_iovec_init(s, len);
}


// Source: http1.c
// Lines 1190-1207
