char *mem_pool_strndup(struct mem_pool *pool, const char *str, size_t len)
{
	char *p = memchr(str, '\0', len);
	size_t actual_len = (p ? p - str : len);
	char *ret = mem_pool_alloc(pool, actual_len+1);

	ret[actual_len] = '\0';
	return memcpy(ret, str, actual_len);
}


// Source: mem-pool.c
// Lines 108-116
