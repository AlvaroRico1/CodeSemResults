static void *pool_alloc_page(git_pool *pool, size_t size)
{
	git_pool_page *page;
	const size_t new_page_size = (size <= pool->page_size) ? pool->page_size : size;
	size_t alloc_size;

	if (GIT_ADD_SIZET_OVERFLOW(&alloc_size, new_page_size, sizeof(git_pool_page)) ||
		!(page = git__malloc(alloc_size)))
		return NULL;

	page->size = new_page_size;
	page->avail = new_page_size - size;
	page->next = pool->pages;

	pool->pages = page;

	return page->data;
}


// Source: pool.c
// Lines 61-78
