static void do_free(void *_self)
{
    struct st_gzip_context_t *self = _self;
    size_t i;

    if (self->zs_is_open) {
        if (self->super.do_transform == do_compress) {
            deflateEnd(&self->zs);
        } else {
            inflateEnd(&self->zs);
        }
    }

    for (i = 0; i != self->bufs.size; ++i)
        free(self->bufs.entries[i].raw);
    free(self->bufs.entries);
    free(self->super.push_buf);
}


// Source: gzip.c
// Lines 143-160
