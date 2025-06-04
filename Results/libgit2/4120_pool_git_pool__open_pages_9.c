uint32_t git_pool__open_pages(git_pool *pool)
{
	uint32_t ct = 0;
	git_pool_page *scan;
	for (scan = pool->pages; scan != NULL; scan = scan->next) ct++;
	return ct;
}


// Source: pool.c
// Lines 94-100
