static int config_backend_cmp(const void *a, const void *b)
{
	const backend_internal *bk_a = (const backend_internal *)(a);
	const backend_internal *bk_b = (const backend_internal *)(b);

	return bk_b->level - bk_a->level;
}


// Source: config.c
// Lines 73-79
