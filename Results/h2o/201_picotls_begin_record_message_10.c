static int begin_record_message(ptls_message_emitter_t *_self)
{
    struct st_ptls_record_message_emitter_t *self = (void *)_self;
    int ret;

    self->rec_start = self->super.buf->off;
    ptls_buffer_push(self->super.buf, PTLS_CONTENT_TYPE_HANDSHAKE, PTLS_RECORD_VERSION_MAJOR, PTLS_RECORD_VERSION_MINOR, 0, 0);
    ret = 0;
Exit:
    return ret;
}


// Source: picotls.c
// Lines 734-744
