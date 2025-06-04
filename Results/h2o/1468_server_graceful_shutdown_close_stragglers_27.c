static void graceful_shutdown_close_stragglers(h2o_timer_t *entry)
{
    h2o_context_t *ctx = H2O_STRUCT_FROM_MEMBER(h2o_context_t, http3._graceful_shutdown_timeout, entry);
    h2o_linklist_t *node, *next;

    /* We've sent two GOAWAY frames, close the remaining connections */
    for (node = ctx->http3._conns.next; node != &ctx->http3._conns; node = next) {
        struct st_h2o_http3_server_conn_t *conn = H2O_STRUCT_FROM_MEMBER(struct st_h2o_http3_server_conn_t, _conns, node);
        next = node->next;
        h2o_quic_close_connection(&conn->h3.super, 0, "shutting down");
    }

    ctx->http3._graceful_shutdown_timeout.cb = NULL;
}


// Source: server.c
// Lines 1903-1916
