static void collect_reqs_of_context(struct st_h2o_status_collector_t *collector, h2o_context_t *ctx)
{
    int i;

    for (i = 0; i < ctx->globalconf->statuses.size; i++) {
        struct st_status_ctx_t *sc = collector->status_ctx.entries + i;
        h2o_status_handler_t *sh = ctx->globalconf->statuses.entries[i];
        if (sc->active && sh->per_thread != NULL)
            sh->per_thread(sc->ctx, ctx);
    }

    if (__sync_sub_and_fetch(&collector->num_remaining_threads_atomic, 1) == 0) {
        struct st_h2o_status_message_t *message = h2o_mem_alloc(sizeof(*message));
        message->super = (h2o_multithread_message_t){{NULL}};
        message->collector = collector;
        h2o_multithread_send_message(collector->src.receiver, &message->super);
    }


// Source: status.c
// Lines 61-77
