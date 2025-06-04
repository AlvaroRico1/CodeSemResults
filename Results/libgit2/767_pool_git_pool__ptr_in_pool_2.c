bool git_pool__ptr_in_pool(git_pool *pool, void *ptr)
{
	git_pool_page *scan;
	for (scan = pool->pages; scan != NULL; scan = scan->next)
		if ((void *)scan->data <= ptr &&
			(void *)(((char *)scan->data) + scan->size) > ptr)
			return true;
	return false;
}


// Source: pool.c
// Lines 102-110
