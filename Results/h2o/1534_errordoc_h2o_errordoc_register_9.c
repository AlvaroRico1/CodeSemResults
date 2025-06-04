void h2o_errordoc_register(h2o_pathconf_t *pathconf, h2o_errordoc_t *errdocs, size_t cnt)
{
    struct st_errordoc_filter_t *self = (void *)h2o_create_filter(pathconf, sizeof(*self));
    size_t i;

    self->super.on_setup_ostream = on_filter_setup_ostream;
    h2o_vector_reserve(NULL, &self->errordocs, cnt);
    self->errordocs.size = cnt;
    for (i = 0; i != cnt; ++i) {
        const h2o_errordoc_t *src = errdocs + i;
        h2o_errordoc_t *dst = self->errordocs.entries + i;
        dst->status = src->status;
        dst->url = h2o_strdup(NULL, src->url.base, src->url.len);
    }


// Source: errordoc.c
// Lines 131-144
