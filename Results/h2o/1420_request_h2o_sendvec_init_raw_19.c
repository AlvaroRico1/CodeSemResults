void h2o_sendvec_init_raw(h2o_sendvec_t *vec, const void *base, size_t len)
{
    static const h2o_sendvec_callbacks_t callbacks = {h2o_sendvec_flatten_raw};
    vec->callbacks = &callbacks;
    vec->raw = (char *)base;
    vec->len = len;
}


// Source: request.c
// Lines 525-531
