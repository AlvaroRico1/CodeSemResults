static inline h2o_iovec_t *alloc_and_init_iovec(h2o_mem_pool_t *pool, const char *base, size_t len)
{
    h2o_iovec_t *iov = h2o_mem_alloc_pool(pool, *iov, 1);
    iov->base = (char *)base;
    iov->len = len;
    return iov;
}


// Source: headers.c
// Lines 42-48
