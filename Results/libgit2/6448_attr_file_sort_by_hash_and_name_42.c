static int sort_by_hash_and_name(const void *a_raw, const void *b_raw)
{
	const git_attr_name *a = a_raw;
	const git_attr_name *b = b_raw;

	if (b->name_hash < a->name_hash)
		return 1;
	else if (b->name_hash > a->name_hash)
		return -1;
	else
		return strcmp(b->name, a->name);
}


// Source: attr_file.c
// Lines 841-852
