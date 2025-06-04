static void on_send(h2o_ostream_t *_self, h2o_req_t *req, h2o_sendvec_t *inbufs, size_t inbufcnt, h2o_send_state_t state)
{
    throttle_resp_t *self = (void *)_self;

    assert(!h2o_timer_is_linked(&self->timeout_entry));

    /* save state */
    h2o_vector_reserve(&req->pool, &self->state.bufs, inbufcnt);
    for (size_t i = 0; i < inbufcnt; ++i) {
        self->state.bufs.entries[i] = inbufs[i];
    }
    self->state.bufs.size = inbufcnt;
    self->state.stream_state = state;

    real_send(self);
}


// Source: throttle_resp.c
// Lines 83-98
