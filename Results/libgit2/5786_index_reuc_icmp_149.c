static int reuc_icmp(const void *a, const void *b)
{
	const git_index_reuc_entry *info_a = a;
	const git_index_reuc_entry *info_b = b;

	return strcasecmp(info_a->path, info_b->path);
}


// Source: index.c
// Lines 312-318
