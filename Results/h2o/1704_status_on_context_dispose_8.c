static void on_context_dispose(h2o_handler_t *_self, h2o_context_t *ctx)
{
    struct st_h2o_root_status_handler_t *self = (void *)_self;
    struct st_h2o_status_context_t *status_ctx = h2o_context_get_handler_context(ctx, &self->super);
    size_t i;

    for (i = 0; i != self->receivers.size; ++i)
        if (self->receivers.entries[i] == &status_ctx->receiver)
            break;
    assert(i != self->receivers.size);
    memmove(self->receivers.entries + i + 1, self->receivers.entries + i, self->receivers.size - i - 1);
    --self->receivers.size;

    h2o_multithread_unregister_receiver(ctx->queue, &status_ctx->receiver);

    free(status_ctx);
}


// Source: status.c
// Lines 244-260
