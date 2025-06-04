static void on_stop(h2o_ostream_t *_self, h2o_req_t *req)
{
    throttle_resp_t *self = (void *)_self;
    if (h2o_timer_is_linked(&self->timeout_entry))
        h2o_timer_unlink(&self->timeout_entry);
}


// Source: throttle_resp.c
// Lines 100-105
