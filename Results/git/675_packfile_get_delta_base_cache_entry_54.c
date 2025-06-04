get_delta_base_cache_entry(struct packed_git *p, off_t base_offset)
{
	struct hashmap_entry entry, *e;
	struct delta_base_cache_key key;

	if (!delta_base_cache.cmpfn)
		return NULL;

	hashmap_entry_init(&entry, pack_entry_hash(p, base_offset));
	key.p = p;
	key.base_offset = base_offset;
	e = hashmap_get(&delta_base_cache, &entry, &key);
	return e ? container_of(e, struct delta_base_cache_entry, ent) : NULL;
}


// Source: packfile.c
// Lines 1370-1383
