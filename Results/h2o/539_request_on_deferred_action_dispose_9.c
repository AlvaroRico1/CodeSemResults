static void on_deferred_action_dispose(void *_action)
{
    struct st_deferred_request_action_t *action = _action;
    h2o_timer_unlink(&action->timeout);
}


// Source: request.c
// Lines 59-63
