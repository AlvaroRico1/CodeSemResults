static void cache_evict_entries(git_cache *cache)
{
	size_t evict_count = git_cache_size(cache) / 2048, i;
	ssize_t evicted_memory = 0;

	if (evict_count < 8)
		evict_count = 8;

	/* do not infinite loop if there's not enough entries to evict  */
	if (evict_count > git_cache_size(cache)) {
		clear_cache(cache);
		return;
	}

	i = 0;
	while (evict_count > 0) {
		git_cached_obj *evict;
		const git_oid *key;

		if (git_oidmap_iterate((void **) &evict, cache->map, &i, &key) == GIT_ITEROVER)
			break;

		evict_count--;
		evicted_memory += evict->size;
		git_oidmap_delete(cache->map, key);
		git_cached_obj_decref(evict);
	}


// Source: cache.c
// Lines 95-121
