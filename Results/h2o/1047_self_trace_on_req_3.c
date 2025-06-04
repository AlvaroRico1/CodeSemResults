static int on_req(h2o_handler_t *handler, h2o_req_t *req)
{
    if (req->conn->callbacks->get_tracer == NULL) {
        h2o_send_error_403(req, "Forbidden", "not available", 0);
        return 0;
    }

    quicly_tracer_t *tracer = req->conn->callbacks->get_tracer(req->conn);
    if (tracer->cb != NULL) {
        h2o_send_error_403(req, "Forbidden", "conn-state handler is already attached", 0);
        return 0;
    }

    /* instantiate the generator */
    struct h2o_self_trace_generator *self = h2o_mem_alloc_shared(&req->pool, sizeof(*self), on_generator_dispose);
    self->super.proceed = do_proceed;
    self->super.stop = NULL;
    self->req = req;
    h2o_buffer_init(&self->buf, &h2o_socket_buffer_prototype);
    h2o_doublebuffer_init(&self->inflight, &h2o_socket_buffer_prototype);
    self->send_timer = (h2o_timer_t){.cb = on_send_timeout};
    self->should_send_buffered = 0;

    /* register */
    tracer->cb = log_trace;
    tracer->ctx = self;

    /* build response headers */
    req->res.status = 200;
    h2o_add_header(&req->pool, &req->res.headers, H2O_TOKEN_CONTENT_TYPE, NULL, H2O_STRLIT("text/plain; charset=utf-8"));
    h2o_add_header(&req->pool, &req->res.headers, H2O_TOKEN_CACHE_CONTROL, NULL, H2O_STRLIT("no-cache, no-store"));
    h2o_buffer_append(&self->buf, "\n", 1); /* add some data for simplicity */

    h2o_start_response(self->req, &self->super);
    do_send(self);

    return 0;
}


// Source: self_trace.c
// Lines 111-148
