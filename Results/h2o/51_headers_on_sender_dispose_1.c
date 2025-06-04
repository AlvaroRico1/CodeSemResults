static void on_sender_dispose(void *_sender)
{
    struct st_headers_early_hints_sender_t *sender = (struct st_headers_early_hints_sender_t *)_sender;
    if (h2o_timer_is_linked(&sender->deferred_timeout_entry))
        h2o_timer_unlink(&sender->deferred_timeout_entry);
}


// Source: headers.c
// Lines 81-86
