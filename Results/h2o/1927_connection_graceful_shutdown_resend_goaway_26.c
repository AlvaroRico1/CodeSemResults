static void graceful_shutdown_resend_goaway(h2o_timer_t *entry)
{
    h2o_context_t *ctx = H2O_STRUCT_FROM_MEMBER(h2o_context_t, http2._graceful_shutdown_timeout, entry);
    h2o_linklist_t *node;
    int do_close_stragglers = 0;

    for (node = ctx->http2._conns.next; node != &ctx->http2._conns; node = node->next) {
        h2o_http2_conn_t *conn = H2O_STRUCT_FROM_MEMBER(h2o_http2_conn_t, _conns, node);
        if (conn->state < H2O_HTTP2_CONN_STATE_HALF_CLOSED) {
            enqueue_goaway(conn, H2O_HTTP2_ERROR_NONE, (h2o_iovec_t){NULL});
            do_close_stragglers = 1;
        }
    }

    /* After waiting a second, we still had active connections. If configured, wait one
     * final timeout before closing the connections */
    if (do_close_stragglers && ctx->globalconf->http2.graceful_shutdown_timeout > 0) {
        ctx->http2._graceful_shutdown_timeout.cb = graceful_shutdown_close_stragglers;
        h2o_timer_link(ctx->loop, ctx->globalconf->http2.graceful_shutdown_timeout, &ctx->http2._graceful_shutdown_timeout);
    }
}


// Source: connection.c
// Lines 96-116
