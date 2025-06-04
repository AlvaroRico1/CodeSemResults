static int begin_raw_message(ptls_message_emitter_t *_self)
{
    struct st_ptls_raw_message_emitter_t *self = (void *)_self;

    self->start_off = self->super.buf->off;
    return 0;
}


// Source: picotls.c
// Lines 5324-5330
