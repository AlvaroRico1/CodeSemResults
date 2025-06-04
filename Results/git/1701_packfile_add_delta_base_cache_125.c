static void add_delta_base_cache(struct packed_git *p, off_t base_offset,
	void *base, unsigned long base_size, enum object_type type)
{
	struct delta_base_cache_entry *ent;
	struct list_head *lru, *tmp;

	/*
	 * Check required to avoid redundant entries when more than one thread
	 * is unpacking the same object, in unpack_entry() (since its phases I
	 * and III might run concurrently across multiple threads).
	 */
	if (in_delta_base_cache(p, base_offset)) {
		free(base);
		return;
	}

	delta_base_cached += base_size;

	list_for_each_safe(lru, tmp, &delta_base_cache_lru) {
		struct delta_base_cache_entry *f =
			list_entry(lru, struct delta_base_cache_entry, lru);
		if (delta_base_cached <= delta_base_cache_limit)
			break;
		release_delta_base_cache(f);
	}

	ent = xmalloc(sizeof(*ent));
	ent->key.p = p;
	ent->key.base_offset = base_offset;
	ent->type = type;
	ent->data = base;
	ent->size = base_size;
	list_add_tail(&ent->lru, &delta_base_cache_lru);

	if (!delta_base_cache.cmpfn)
		hashmap_init(&delta_base_cache, delta_base_cache_hash_cmp, NULL, 0);
	hashmap_entry_init(&ent->ent, pack_entry_hash(p, base_offset));
	hashmap_add(&delta_base_cache, &ent->ent);
}


// Source: packfile.c
// Lines 1459-1497
