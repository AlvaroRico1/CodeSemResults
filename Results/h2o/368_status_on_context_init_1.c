static void on_context_init(h2o_handler_t *_self, h2o_context_t *ctx)
{
    struct st_h2o_root_status_handler_t *self = (void *)_self;
    struct st_h2o_status_context_t *status_ctx = h2o_mem_alloc(sizeof(*status_ctx));

    status_ctx->ctx = ctx;
    h2o_multithread_register_receiver(ctx->queue, &status_ctx->receiver, on_collect_notify);

    h2o_vector_reserve(NULL, &self->receivers, self->receivers.size + 1);
    self->receivers.entries[self->receivers.size++] = &status_ctx->receiver;

    h2o_context_set_handler_context(ctx, &self->super, status_ctx);
}


// Source: status.c
// Lines 230-242
