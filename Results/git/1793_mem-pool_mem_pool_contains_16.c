int mem_pool_contains(struct mem_pool *pool, void *mem)
{
	struct mp_block *p;

	/* Check if memory is allocated in a block */
	for (p = pool->mp_block; p; p = p->next_block)
		if ((mem >= ((void *)p->space)) &&
		    (mem < ((void *)p->end)))
			return 1;

	return 0;
}


// Source: mem-pool.c
// Lines 118-129
