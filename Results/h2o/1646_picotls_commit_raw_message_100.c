static int commit_raw_message(ptls_message_emitter_t *_self)
{
    struct st_ptls_raw_message_emitter_t *self = (void *)_self;
    size_t epoch;

    /* epoch is the key epoch, with the only exception being 2nd CH generated after 0-RTT key */
    epoch = self->super.enc->epoch;
    if (epoch == 1 && self->super.buf->base[self->start_off] == PTLS_HANDSHAKE_TYPE_CLIENT_HELLO)
        epoch = 0;

    for (++epoch; epoch < 5; ++epoch) {
        assert(self->epoch_offsets[epoch] == self->start_off);
        self->epoch_offsets[epoch] = self->super.buf->off;
    }

    self->start_off = SIZE_MAX;

    return 0;
}


// Source: picotls.c
// Lines 5332-5350
