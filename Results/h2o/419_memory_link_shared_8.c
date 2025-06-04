static void link_shared(h2o_mem_pool_t *pool, struct st_h2o_mem_pool_shared_entry_t *entry)
{
    struct st_h2o_mem_pool_shared_ref_t *ref = h2o_mem_alloc_pool(pool, *ref, 1);
    ref->entry = entry;
    ref->next = pool->shared_refs;
    pool->shared_refs = ref;
}


// Source: memory.c
// Lines 211-217
