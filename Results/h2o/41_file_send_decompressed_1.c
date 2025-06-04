static void send_decompressed(h2o_ostream_t *_self, h2o_req_t *req, h2o_sendvec_t *inbufs, size_t inbufcnt, h2o_send_state_t state)
{
    if (inbufcnt == 0 && h2o_send_state_is_in_progress(state)) {
        h2o_ostream_send_next(_self, req, inbufs, inbufcnt, state);
        return;
    }

    struct st_gzip_decompress_t *self = (void *)_self;
    h2o_sendvec_t *outbufs;
    size_t outbufcnt;

    state = h2o_compress_transform(self->decompressor, req, inbufs, inbufcnt, state, &outbufs, &outbufcnt);
    h2o_ostream_send_next(&self->super, req, outbufs, outbufcnt, state);
}


// Source: file.c
// Lines 318-331
