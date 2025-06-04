static unsigned long free_unpacked(struct unpacked *n)
{
	unsigned long freed_mem = sizeof_delta_index(n->index);
	free_delta_index(n->index);
	n->index = NULL;
	if (n->data) {
		freed_mem += SIZE(n->entry);
		FREE_AND_NULL(n->data);
	}


// Source: pack-objects.c
// Lines 2608-2616
