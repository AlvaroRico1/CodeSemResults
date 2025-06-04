void *h2o_mem__do_alloc_pool_aligned(h2o_mem_pool_t *pool, size_t alignment, size_t sz)
{
#define ALIGN_TO(x, a) (((x) + (a)-1) & ~((a)-1))
    void *ret;

    if (sz >= (sizeof(pool->chunks->bytes) - sizeof(pool->chunks->next)) / 4) {
        /* allocate large requests directly */
        struct st_h2o_mem_pool_direct_t *newp = h2o_mem_alloc(offsetof(struct st_h2o_mem_pool_direct_t, bytes) + sz);
        newp->next = pool->directs;
        pool->directs = newp;
        return newp->bytes;
    }


// Source: memory.c
// Lines 179-190
