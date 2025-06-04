static void graceful_shutdown_close_stragglers(h2o_timer_t *entry)
{
    h2o_context_t *ctx = H2O_STRUCT_FROM_MEMBER(h2o_context_t, http2._graceful_shutdown_timeout, entry);
    h2o_linklist_t *node, *next;

    /* We've sent two GOAWAY frames, close the remaining connections */
    for (node = ctx->http2._conns.next; node != &ctx->http2._conns; node = next) {
        h2o_http2_conn_t *conn = H2O_STRUCT_FROM_MEMBER(h2o_http2_conn_t, _conns, node);
        next = node->next;
        close_connection(conn);
    }
}


// Source: connection.c
// Lines 83-94
