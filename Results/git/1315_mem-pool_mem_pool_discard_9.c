void mem_pool_discard(struct mem_pool *pool, int invalidate_memory)
{
	struct mp_block *block, *block_to_free;

	block = pool->mp_block;
	while (block)
	{
		block_to_free = block;
		block = block->next_block;

		if (invalidate_memory)
			memset(block_to_free->space, 0xDD, ((char *)block_to_free->end) - ((char *)block_to_free->space));

		free(block_to_free);
	}

	pool->mp_block = NULL;
	pool->pool_alloc = 0;
}


// Source: mem-pool.c
// Lines 47-65
