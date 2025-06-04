static struct mp_block *mem_pool_alloc_block(struct mem_pool *pool,
					     size_t block_alloc,
					     struct mp_block *insert_after)
{
	struct mp_block *p;

	pool->pool_alloc += sizeof(struct mp_block) + block_alloc;
	p = xmalloc(st_add(sizeof(struct mp_block), block_alloc));

	p->next_free = (char *)p->space;
	p->end = p->next_free + block_alloc;

	if (insert_after) {
		p->next_block = insert_after->next_block;
		insert_after->next_block = p;
	} else {
		p->next_block = pool->mp_block;
		pool->mp_block = p;
	}

	return p;
}


// Source: mem-pool.c
// Lines 15-36
