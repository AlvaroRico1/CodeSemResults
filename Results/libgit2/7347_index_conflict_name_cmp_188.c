static int conflict_name_cmp(const void *a, const void *b)
{
	const git_index_name_entry *name_a = a;
	const git_index_name_entry *name_b = b;

	if (name_a->ancestor && !name_b->ancestor)
		return 1;

	if (!name_a->ancestor && name_b->ancestor)
		return -1;

	if (name_a->ancestor)
		return strcmp(name_a->ancestor, name_b->ancestor);

	if (!name_a->ours || !name_b->ours)
		return 0;

	return strcmp(name_a->ours, name_b->ours);
}


// Source: index.c
// Lines 245-263
