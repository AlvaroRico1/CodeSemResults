static void on_collect_notify(h2o_multithread_receiver_t *receiver, h2o_linklist_t *messages)
{
    struct st_h2o_status_context_t *status_ctx = H2O_STRUCT_FROM_MEMBER(struct st_h2o_status_context_t, receiver, receiver);

    while (!h2o_linklist_is_empty(messages)) {
        struct st_h2o_status_message_t *message = H2O_STRUCT_FROM_MEMBER(struct st_h2o_status_message_t, super, messages->next);
        struct st_h2o_status_collector_t *collector = message->collector;
        h2o_linklist_unlink(&message->super.link);
        free(message);

        if (__sync_add_and_fetch(&collector->num_remaining_threads_atomic, 0) != 0) {
            collect_reqs_of_context(collector, status_ctx->ctx);
        } else {
            send_response(collector);
        }
    }


// Source: status.c
// Lines 125-140
