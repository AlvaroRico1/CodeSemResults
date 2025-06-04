static int path_hashmap_cmp(const void *cmp_data,
			    const struct hashmap_entry *eptr,
			    const struct hashmap_entry *entry_or_key,
			    const void *keydata)
{
	const struct path_hashmap_entry *a, *b;
	const char *key = keydata;

	a = container_of(eptr, const struct path_hashmap_entry, e);
	b = container_of(entry_or_key, const struct path_hashmap_entry, e);

	return fspathcmp(a->path, key ? key : b->path);
}


// Source: merge-recursive.c
// Lines 48-60
