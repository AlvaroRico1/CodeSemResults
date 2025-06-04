void h2o_mem_free_recycle(h2o_mem_recycle_t *allocator, void *p)
{
#if !ASAN_IN_USE
    /* register the pointer to the pool and return unless the pool is full */
    if (allocator->cnt < allocator->max) {
        struct st_h2o_mem_recycle_chunk_t *chunk = p;
        chunk->next = allocator->_link;
        allocator->_link = chunk;
        ++allocator->cnt;
        return;
    }


// Source: memory.c
// Lines 112-122
