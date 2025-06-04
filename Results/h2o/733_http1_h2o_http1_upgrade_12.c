void h2o_http1_upgrade(h2o_req_t *req, h2o_iovec_t *inbufs, size_t inbufcnt, h2o_http1_upgrade_cb on_complete, void *user_data)
{

    assert(conn_is_h1(req->conn));
    struct st_h2o_http1_conn_t *conn = (void *)req->conn;

    h2o_iovec_t *bufs = alloca(sizeof(h2o_iovec_t) * (inbufcnt + 1));

    conn->upgrade.data = user_data;
    conn->upgrade.cb = on_complete;

    bufs[0].base = h2o_mem_alloc_pool(
        &conn->req.pool, char,
        flatten_headers_estimate_size(&conn->req, conn->super.ctx->globalconf->server_name.len + sizeof("upgrade") - 1));
    bufs[0].len = flatten_headers(bufs[0].base, &conn->req, conn->req.res.status == 101 ? "upgrade" : "close");
    h2o_memcpy(bufs + 1, inbufs, sizeof(h2o_iovec_t) * inbufcnt);

    h2o_socket_write(conn->sock, bufs, inbufcnt + 1, on_upgrade_complete);
}


// Source: http1.c
// Lines 1274-1292
