static void *pool_alloc(git_pool *pool, size_t size)
{
	git_pool_page *page = pool->pages;
	void *ptr = NULL;

	if (!page || page->avail < size)
		return pool_alloc_page(pool, size);

	ptr = &page->data[page->size - page->avail];
	page->avail -= size;

	return ptr;
}


// Source: pool.c
// Lines 80-92
