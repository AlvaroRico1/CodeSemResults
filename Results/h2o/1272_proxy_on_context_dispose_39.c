static void on_context_dispose(h2o_handler_t *_self, h2o_context_t *ctx)
{
    struct rp_handler_t *self = (void *)_self;
    struct rp_handler_context_t *handler_ctx = h2o_context_get_handler_context(ctx, &self->super);

    if (handler_ctx->client_ctx != NULL) {
        if (handler_ctx->client_ctx->http3 != NULL)
            destroy_http3_context(handler_ctx->client_ctx->http3);
        free(handler_ctx->client_ctx);
    }

    h2o_socketpool_unregister_loop(self->sockpool, ctx->loop);
}


// Source: proxy.c
// Lines 166-178
