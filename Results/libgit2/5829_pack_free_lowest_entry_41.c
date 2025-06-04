static void free_lowest_entry(git_pack_cache *cache)
{
	off64_t offset;
	git_pack_cache_entry *entry;

	git_offmap_foreach(cache->entries, offset, entry, {
		if (entry && git_atomic32_get(&entry->refcount) == 0) {
			cache->memory_used -= entry->raw.len;
			git_offmap_delete(cache->entries, offset);
			free_cache_object(entry);
		}
	});
}


// Source: pack.c
// Lines 126-138
