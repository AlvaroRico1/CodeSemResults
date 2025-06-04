static int commit_record_message(ptls_message_emitter_t *_self)
{
    struct st_ptls_record_message_emitter_t *self = (void *)_self;
    int ret;

    if (self->super.enc->aead != NULL) {
        ret = buffer_encrypt_record(self->super.buf, self->rec_start, self->super.enc);
    } else {
        /* TODO allow CH,SH,HRR above 16KB */
        size_t sz = self->super.buf->off - self->rec_start - 5;
        assert(sz <= PTLS_MAX_PLAINTEXT_RECORD_SIZE);
        self->super.buf->base[self->rec_start + 3] = (uint8_t)(sz >> 8);
        self->super.buf->base[self->rec_start + 4] = (uint8_t)(sz);
        ret = 0;
    }

    return ret;
}


// Source: picotls.c
// Lines 746-763
