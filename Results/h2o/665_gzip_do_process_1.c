static h2o_send_state_t do_process(h2o_compress_context_t *_self, h2o_sendvec_t *inbufs, size_t inbufcnt, h2o_send_state_t state,
                                   h2o_sendvec_t **outbufs, size_t *outbufcnt, processor proc)
{
    struct st_gzip_context_t *self = (void *)_self;
    size_t outbufindex;
    h2o_sendvec_t *last_buf;

    outbufindex = 0;
    self->bufs.entries[0].len = 0;

    if (inbufcnt != 0) {
        size_t i;
        for (i = 0; i != inbufcnt - 1; ++i) {
            assert(inbufs[i].callbacks->flatten == h2o_sendvec_flatten_raw);
            outbufindex = process_chunk(self, inbufs[i].raw, inbufs[i].len, Z_NO_FLUSH, outbufindex, proc);
        }
        assert(inbufs[i].callbacks->flatten == h2o_sendvec_flatten_raw);
        last_buf = inbufs + i;
    } else {
        static const h2o_sendvec_t zero_buf = {0};
        last_buf = (h2o_sendvec_t *)&zero_buf;
    }


// Source: gzip.c
// Lines 91-112
