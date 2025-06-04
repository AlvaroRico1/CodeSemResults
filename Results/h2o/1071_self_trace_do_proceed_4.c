static void do_proceed(h2o_generator_t *_self, h2o_req_t *_req)
{
    struct h2o_self_trace_generator *self = (void *)_self;

    assert(self->inflight.inflight);
    h2o_doublebuffer_consume(&self->inflight);

    adjust_send_timer(self);
}


// Source: self_trace.c
// Lines 101-109
