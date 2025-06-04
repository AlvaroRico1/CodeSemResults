static struct diff_filespec *pool_alloc_filespec(struct mem_pool *pool,
						 const char *path)
{
	/* Similar to alloc_filespec(), but allocate from pool and reuse path */
	struct diff_filespec *spec;

	spec = mem_pool_calloc(pool, 1, sizeof(*spec));
	spec->path = (char*)path; /* spec won't modify it */

	spec->count = 1;
	spec->is_binary = -1;
	return spec;
}


// Source: merge-ort.c
// Lines 650-662
