void git_pool_clear(git_pool *pool)
{
	git_pool_page *scan, *next;

	for (scan = pool->pages; scan != NULL; scan = next) {
		next = scan->next;
		git__free(scan);
	}

	pool->pages = NULL;
}


// Source: pool.c
// Lines 49-59
