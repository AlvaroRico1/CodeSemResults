void *mem_pool_alloc(struct mem_pool *pool, size_t len)
{
	struct mp_block *p = NULL;
	void *r;

	/* round up to a 'uintmax_t' alignment */
	if (len & (sizeof(uintmax_t) - 1))
		len += sizeof(uintmax_t) - (len & (sizeof(uintmax_t) - 1));

	if (pool->mp_block &&
	    pool->mp_block->end - pool->mp_block->next_free >= len)
		p = pool->mp_block;

	if (!p) {
		if (len >= (pool->block_alloc / 2))
			return mem_pool_alloc_block(pool, len, pool->mp_block);

		p = mem_pool_alloc_block(pool, pool->block_alloc, NULL);
	}

	r = p->next_free;
	p->next_free += len;
	return r;
}


// Source: mem-pool.c
// Lines 67-90
