static void on_handler_dispose(h2o_handler_t *_handler)
{
    h2o_fastcgi_handler_t *handler = (void *)_handler;

    if (handler->config.callbacks.dispose != NULL)
        handler->config.callbacks.dispose(handler, handler->config.callbacks.data);

    h2o_socketpool_dispose(&handler->sockpool);
    free(handler->config.document_root.base);
}


// Source: fastcgi.c
// Lines 820-829
