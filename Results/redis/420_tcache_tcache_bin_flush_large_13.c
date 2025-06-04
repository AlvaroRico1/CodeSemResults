tcache_bin_flush_large(tsd_t *tsd, cache_bin_t *tbin, szind_t binind,
    unsigned rem, tcache_t *tcache) {
	bool merged_stats = false;

	assert(binind < nhbins);
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
		/* Lock the arena associated with the first object. */
		extent_t *extent = item_extent[0];
		arena_t *locked_arena = extent_arena_get(extent);
		UNUSED bool idump;

		if (config_prof) {
			idump = false;
		}

		malloc_mutex_lock(tsd_tsdn(tsd), &locked_arena->large_mtx);
		for (unsigned i = 0; i < nflush; i++) {
			void *ptr = *(tbin->avail - 1 - i);
			assert(ptr != NULL);
			extent = item_extent[i];
			if (extent_arena_get(extent) == locked_arena) {
				large_dalloc_prep_junked_locked(tsd_tsdn(tsd),
				    extent);
			}
		}
		if ((config_prof || config_stats) && locked_arena == arena) {
			if (config_prof) {
				idump = arena_prof_accum(tsd_tsdn(tsd), arena,
				    tcache->prof_accumbytes);
				tcache->prof_accumbytes = 0;
			}
			if (config_stats) {
				merged_stats = true;
				arena_stats_large_nrequests_add(tsd_tsdn(tsd),
				    &arena->stats, binind,
				    tbin->tstats.nrequests);
				tbin->tstats.nrequests = 0;
			}
		}
		malloc_mutex_unlock(tsd_tsdn(tsd), &locked_arena->large_mtx);

		unsigned ndeferred = 0;
		for (unsigned i = 0; i < nflush; i++) {
			void *ptr = *(tbin->avail - 1 - i);
			extent = item_extent[i];
			assert(ptr != NULL && extent != NULL);

			if (extent_arena_get(extent) == locked_arena) {
				large_dalloc_finish(tsd_tsdn(tsd), extent);
			} else {
				/*
				 * This object was allocated via a different
				 * arena than the one that is currently locked.
				 * Stash the object, so that it can be handled
				 * in a future pass.
				 */
				*(tbin->avail - 1 - ndeferred) = ptr;
				item_extent[ndeferred] = extent;
				ndeferred++;
			}
		}
		if (config_prof && idump) {
			prof_idump(tsd_tsdn(tsd));
		}
		arena_decay_ticks(tsd_tsdn(tsd), locked_arena, nflush -
		    ndeferred);
		nflush = ndeferred;
	}


// Source: tcache.c
// Lines 189-267
