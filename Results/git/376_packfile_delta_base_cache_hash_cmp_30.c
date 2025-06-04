static int delta_base_cache_hash_cmp(const void *unused_cmp_data,
				     const struct hashmap_entry *va,
				     const struct hashmap_entry *vb,
				     const void *vkey)
{
	const struct delta_base_cache_entry *a, *b;
	const struct delta_base_cache_key *key = vkey;

	a = container_of(va, const struct delta_base_cache_entry, ent);
	b = container_of(vb, const struct delta_base_cache_entry, ent);

	if (key)
		return !delta_base_cache_key_eq(&a->key, key);
	else
		return !delta_base_cache_key_eq(&a->key, &b->key);
}


// Source: packfile.c
// Lines 1391-1406
