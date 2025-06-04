static struct st_deferred_request_action_t *create_deferred_action(h2o_req_t *req, size_t sz, h2o_timer_cb cb)
{
    struct st_deferred_request_action_t *action = h2o_mem_alloc_shared(&req->pool, sz, on_deferred_action_dispose);
    action->req = req;
    h2o_timer_init(&action->timeout, cb);
    h2o_timer_link(req->conn->ctx->loop, 0, &action->timeout);
    return action;
}


// Source: request.c
// Lines 65-72
