tcache_arena_dissociate(tsdn_t *tsdn, tcache_t *tcache) {
	arena_t *arena = tcache->arena;
	assert(arena != NULL);
	if (config_stats) {
		/* Unlink from list of extant tcaches. */
		malloc_mutex_lock(tsdn, &arena->tcache_ql_mtx);
		if (config_debug) {
			bool in_ql = false;
			tcache_t *iter;
			ql_foreach(iter, &arena->tcache_ql, link) {
				if (iter == tcache) {
					in_ql = true;
					break;
				}
			}
			assert(in_ql);
		}
		ql_remove(&arena->tcache_ql, tcache, link);
		ql_remove(&arena->cache_bin_array_descriptor_ql,
		    &tcache->cache_bin_array_descriptor, link);
		tcache_stats_merge(tsdn, tcache, arena);
		malloc_mutex_unlock(tsdn, &arena->tcache_ql_mtx);
	}
	tcache->arena = NULL;
}


// Source: tcache.c
// Lines 308-332
