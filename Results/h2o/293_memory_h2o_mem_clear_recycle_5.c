void h2o_mem_clear_recycle(h2o_mem_recycle_t *allocator, int full)
{
    struct st_h2o_mem_recycle_chunk_t *chunk;

    if (allocator->cnt != 0) {
        do {
            chunk = allocator->_link;
            allocator->_link = allocator->_link->next;
            free(chunk);
            --allocator->cnt;
        } while (full && allocator->cnt != 0);
        assert((allocator->cnt != 0) == (allocator->_link != NULL));
    }
}


// Source: memory.c
// Lines 128-141
