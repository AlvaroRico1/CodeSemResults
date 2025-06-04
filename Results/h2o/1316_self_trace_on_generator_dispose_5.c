static void on_generator_dispose(void *_self)
{
    struct h2o_self_trace_generator *self = _self;

    quicly_tracer_t *tracer = self->req->conn->callbacks->get_tracer(self->req->conn);
    tracer->cb = NULL;
    tracer->ctx = NULL;

    h2o_buffer_dispose(&self->buf);
    h2o_doublebuffer_dispose(&self->inflight);
    h2o_timer_unlink(&self->send_timer);
}


// Source: self_trace.c
// Lines 37-48
