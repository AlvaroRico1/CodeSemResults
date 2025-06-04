void *h2o_mem_alloc_recycle(h2o_mem_recycle_t *allocator, size_t sz)
{
    struct st_h2o_mem_recycle_chunk_t *chunk;
    if (allocator->cnt == 0)
        return h2o_mem_alloc(sz);
    /* detach and return the pooled pointer */
    chunk = allocator->_link;
    assert(chunk != NULL);
    allocator->_link = chunk->next;
    --allocator->cnt;
    return chunk;
}


// Source: memory.c
// Lines 99-110
