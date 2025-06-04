static void on_context_dispose(h2o_handler_t *_handler, h2o_context_t *ctx)
{
    h2o_fastcgi_handler_t *handler = (void *)_handler;
    h2o_socketpool_unregister_loop(&handler->sockpool, ctx->loop);
}


// Source: fastcgi.c
// Lines 814-818
