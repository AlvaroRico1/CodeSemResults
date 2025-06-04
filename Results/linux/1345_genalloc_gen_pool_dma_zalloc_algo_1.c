void *gen_pool_dma_zalloc_algo(struct gen_pool *pool, size_t size,
		dma_addr_t *dma, genpool_algo_t algo, void *data)
{
	void *vaddr = gen_pool_dma_alloc_algo(pool, size, dma, algo, data);

	if (vaddr)
		memset(vaddr, 0, size);

	return vaddr;
}


// Source: genalloc.c
// Lines 438-447
