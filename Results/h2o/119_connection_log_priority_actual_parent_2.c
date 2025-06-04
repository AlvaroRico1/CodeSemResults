static h2o_iovec_t log_priority_actual_parent(h2o_req_t *req)
{
    h2o_http2_conn_t *conn = (void *)req->conn;
    h2o_http2_stream_t *stream = H2O_STRUCT_FROM_MEMBER(h2o_http2_stream_t, req, req);
    char *s = h2o_mem_alloc_pool(&stream->req.pool, *s, sizeof(H2O_UINT32_LONGEST_STR));
    size_t len = (size_t)sprintf(s, "%" PRIu32, get_parent_stream_id(conn, stream));
    return h2o_iovec_init(s, len);
}


// Source: connection.c
// Lines 1661-1668
