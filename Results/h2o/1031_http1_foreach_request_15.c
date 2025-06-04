static int foreach_request(h2o_context_t *ctx, int (*cb)(h2o_req_t *req, void *cbdata), void *cbdata)
{
    h2o_linklist_t *node;

    for (node = ctx->http1._conns.next; node != &ctx->http1._conns; node = node->next) {
        struct st_h2o_http1_conn_t *conn = H2O_STRUCT_FROM_MEMBER(struct st_h2o_http1_conn_t, _conns, node);
        int ret = cb(&conn->req, cbdata);
        if (ret != 0)
            return ret;
    }
    return 0;
}


// Source: http1.c
// Lines 1209-1220
