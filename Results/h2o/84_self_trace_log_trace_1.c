static void log_trace(void *_self, const char *fmt, ...)
{
    struct h2o_self_trace_generator *self = _self;

    /* append provided input to the buffer */
    if (self->buf->size < BUFFER_LIMIT) {
        va_list args;
        va_start(args, fmt);

        h2o_iovec_t buf = h2o_buffer_reserve(&self->buf, 1024);
        int len = vsnprintf(buf.base, buf.len, fmt, args);
        if (len >= buf.len) {
            buf = h2o_buffer_reserve(&self->buf, len + 1);
            len = vsnprintf(buf.base, buf.len, fmt, args);
            assert(len < buf.len);
        }
        self->buf->size += len;

        va_end(args);
    }

    /* Log is sent only when there's another request inflight. Otherwise, it is buffered until another request becomes inflight. */
    if (!self->should_send_buffered && self->req->conn->callbacks->num_reqs_inflight(self->req->conn) > 1)
        self->should_send_buffered = 1;
    adjust_send_timer(self);
}


// Source: self_trace.c
// Lines 74-99
