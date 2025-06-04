static void do_send(h2o_ostream_t *_self, h2o_req_t *req, h2o_sendvec_t *inbufs, size_t inbufcnt, h2o_send_state_t state)
{
    struct st_compress_encoder_t *self = (void *)_self;
    h2o_sendvec_t *outbufs;
    size_t outbufcnt;

    if (inbufcnt == 0 && h2o_send_state_is_in_progress(state)) {
        h2o_ostream_send_next(&self->super, req, inbufs, inbufcnt, state);
        return;
    }

    state = h2o_compress_transform(self->compressor, req, inbufs, inbufcnt, state, &outbufs, &outbufcnt);
    h2o_ostream_send_next(&self->super, req, outbufs, outbufcnt, state);
}


// Source: compress.c
// Lines 40-53
