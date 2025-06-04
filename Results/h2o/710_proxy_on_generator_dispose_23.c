static void on_generator_dispose(void *_self)
{
    struct rp_generator_t *self = _self;
    do_close(self);

    if (self->last_content_before_send != NULL) {
        h2o_buffer_dispose(&self->last_content_before_send);
    }
    h2o_doublebuffer_dispose(&self->sending);
}


// Source: proxy.c
// Lines 661-670
