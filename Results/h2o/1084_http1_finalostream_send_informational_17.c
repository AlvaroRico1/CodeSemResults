static void finalostream_send_informational(h2o_ostream_t *_self, h2o_req_t *req)
{
    struct st_h2o_http1_conn_t *conn = (struct st_h2o_http1_conn_t *)req->conn;
    assert(_self == &conn->_ostr_final.super);

    size_t len = sizeof("HTTP/1.1  \r\n\r\n") + 3 + strlen(req->res.reason) - 1;
    h2o_iovec_t buf = h2o_iovec_init(NULL, len);

    int i;
    for (i = 0; i != req->res.headers.size; ++i)
        buf.len += req->res.headers.entries[i].name->len + req->res.headers.entries[i].value.len + 4;

    buf.base = h2o_mem_alloc_pool(&req->pool, char, buf.len);
    char *dst = buf.base;
    dst += sprintf(dst, "HTTP/1.1 %d %s\r\n", req->res.status, req->res.reason);
    dst += flatten_res_headers(dst, req);
    *dst++ = '\r';
    *dst++ = '\n';

    h2o_vector_reserve(&req->pool, &conn->_ostr_final.informational.pending, conn->_ostr_final.informational.pending.size + 1);
    conn->_ostr_final.informational.pending.entries[conn->_ostr_final.informational.pending.size++] = buf;

    if (!conn->_ostr_final.informational.write_inflight)
        do_send_informational(conn);
}


// Source: http1.c
// Lines 1131-1155
