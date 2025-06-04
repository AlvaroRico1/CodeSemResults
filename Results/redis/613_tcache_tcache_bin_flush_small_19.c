tcache_bin_flush_small(tsd_t *tsd, tcache_t *tcache, cache_bin_t *tbin,
    szind_t binind, unsigned rem) {
	bool merged_stats = false;

	assert(binind < NBINS);
	assert((cache_bin_sz_t)rem <= tbin->ncached);

	arena_t *arena = tcache->arena;
	assert(arena != NULL);
	unsigned nflush = tbin->ncached - rem;
	VARIABLE_ARRAY(extent_t *, item_extent, nflush);
	/* Look up extent once per item. */
	for (unsigned i = 0 ; i < nflush; i++) {
		item_extent[i] = iealloc(tsd_tsdn(tsd), *(tbin->avail - 1 - i));
	}

	while (nflush > 0) {
		/* Lock the arena bin associated with the first object. */
		extent_t *extent = item_extent[0];
		arena_t *bin_arena = extent_arena_get(extent);
		bin_t *bin = &bin_arena->bins[binind];

		if (config_prof && bin_arena == arena) {
			if (arena_prof_accum(tsd_tsdn(tsd), arena,
			    tcache->prof_accumbytes)) {
				prof_idump(tsd_tsdn(tsd));
			}
			tcache->prof_accumbytes = 0;
		}

		malloc_mutex_lock(tsd_tsdn(tsd), &bin->lock);
		if (config_stats && bin_arena == arena) {
			assert(!merged_stats);
			merged_stats = true;
			bin->stats.nflushes++;
			bin->stats.nrequests += tbin->tstats.nrequests;
			tbin->tstats.nrequests = 0;
		}
		unsigned ndeferred = 0;
		for (unsigned i = 0; i < nflush; i++) {
			void *ptr = *(tbin->avail - 1 - i);
			extent = item_extent[i];
			assert(ptr != NULL && extent != NULL);

			if (extent_arena_get(extent) == bin_arena) {
				arena_dalloc_bin_junked_locked(tsd_tsdn(tsd),
				    bin_arena, extent, ptr);
			} else {
				/*
				 * This object was allocated via a different
				 * arena bin than the one that is currently
				 * locked.  Stash the object, so that it can be
				 * handled in a future pass.
				 */
				*(tbin->avail - 1 - ndeferred) = ptr;
				item_extent[ndeferred] = extent;
				ndeferred++;
			}
		}


// Source: tcache.c
// Lines 104-162
