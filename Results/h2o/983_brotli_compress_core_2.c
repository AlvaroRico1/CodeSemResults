static void compress_core(struct st_brotli_context_t *self, BrotliEncoderOperation op, const uint8_t **src, size_t *srclen)
{
    size_t bufindex = self->bufs.size - 1;

    if (self->bufs.entries[bufindex].len == self->buf_capacity) {
        expand_buf(self);
        ++bufindex;
    }
    uint8_t *dst = (uint8_t *)self->bufs.entries[bufindex].raw + self->bufs.entries[bufindex].len;
    size_t dstlen = self->buf_capacity - self->bufs.entries[bufindex].len;

    if (!BrotliEncoderCompressStream(self->state, op, srclen, src, &dstlen, &dst, NULL))
        h2o_fatal("BrotliEncoderCompressStream");

    self->bufs.entries[bufindex].len = self->buf_capacity - dstlen;
}


// Source: brotli.c
// Lines 46-61
