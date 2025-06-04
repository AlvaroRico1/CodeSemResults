int git_index_entry_icmp(const void *a, const void *b)
{
	int diff;
	const git_index_entry *entry_a = a;
	const git_index_entry *entry_b = b;

	diff = strcasecmp(entry_a->path, entry_b->path);

	if (diff == 0)
		diff = (GIT_INDEX_ENTRY_STAGE(entry_a) - GIT_INDEX_ENTRY_STAGE(entry_b));

	return diff;
}


// Source: index.c
// Lines 231-243
