static int backend_sort_cmp(const void *a, const void *b)
{
	const backend_internal *backend_a = (const backend_internal *)(a);
	const backend_internal *backend_b = (const backend_internal *)(b);

	if (backend_b->priority == backend_a->priority) {
		if (backend_a->is_alternate)
			return -1;
		if (backend_b->is_alternate)
			return 1;
		return 0;
	}
	return (backend_b->priority - backend_a->priority);
}


// Source: odb.c
// Lines 430-443
