extent_alloc_dss(tsdn_t *tsdn, arena_t *arena, void *new_addr, size_t size,
    size_t alignment, bool *zero, bool *commit) {
	extent_t *gap;

	cassert(have_dss);
	assert(size > 0);
	assert(alignment > 0);

	/*
	 * sbrk() uses a signed increment argument, so take care not to
	 * interpret a large allocation request as a negative increment.
	 */
	if ((intptr_t)size < 0) {
		return NULL;
	}

	gap = extent_alloc(tsdn, arena);
	if (gap == NULL) {
		return NULL;
	}

	extent_dss_extending_start();
	if (!atomic_load_b(&dss_exhausted, ATOMIC_ACQUIRE)) {
		/*
		 * The loop is necessary to recover from races with other
		 * threads that are using the DSS for something other than
		 * malloc.
		 */
		while (true) {
			void *max_cur = extent_dss_max_update(new_addr);
			if (max_cur == NULL) {
				goto label_oom;
			}

			/*
			 * Compute how much page-aligned gap space (if any) is
			 * necessary to satisfy alignment.  This space can be
			 * recycled for later use.
			 */
			void *gap_addr_page = (void *)(PAGE_CEILING(
			    (uintptr_t)max_cur));
			void *ret = (void *)ALIGNMENT_CEILING(
			    (uintptr_t)gap_addr_page, alignment);
			size_t gap_size_page = (uintptr_t)ret -
			    (uintptr_t)gap_addr_page;
			if (gap_size_page != 0) {
				extent_init(gap, arena, gap_addr_page,
				    gap_size_page, false, NSIZES,
				    arena_extent_sn_next(arena),
				    extent_state_active, false, true, true);
			}
			/*
			 * Compute the address just past the end of the desired
			 * allocation space.
			 */
			void *dss_next = (void *)((uintptr_t)ret + size);
			if ((uintptr_t)ret < (uintptr_t)max_cur ||
			    (uintptr_t)dss_next < (uintptr_t)max_cur) {
				goto label_oom; /* Wrap-around. */
			}
			/* Compute the increment, including subpage bytes. */
			void *gap_addr_subpage = max_cur;
			size_t gap_size_subpage = (uintptr_t)ret -
			    (uintptr_t)gap_addr_subpage;
			intptr_t incr = gap_size_subpage + size;

			assert((uintptr_t)max_cur + incr == (uintptr_t)ret +
			    size);

			/* Try to allocate. */
			void *dss_prev = extent_dss_sbrk(incr);
			if (dss_prev == max_cur) {
				/* Success. */
				atomic_store_p(&dss_max, dss_next,
				    ATOMIC_RELEASE);
				extent_dss_extending_finish();

				if (gap_size_page != 0) {
					extent_dalloc_gap(tsdn, arena, gap);
				} else {
					extent_dalloc(tsdn, arena, gap);
				}
				if (!*commit) {
					*commit = pages_decommit(ret, size);
				}
				if (*zero && *commit) {
					extent_hooks_t *extent_hooks =
					    EXTENT_HOOKS_INITIALIZER;
					extent_t extent;

					extent_init(&extent, arena, ret, size,
					    size, false, NSIZES,
					    extent_state_active, false, true,
					    true);
					if (extent_purge_forced_wrapper(tsdn,
					    arena, &extent_hooks, &extent, 0,
					    size)) {
						memset(ret, 0, size);
					}
				}
				return ret;
			}
			/*
			 * Failure, whether due to OOM or a race with a raw
			 * sbrk() call from outside the allocator.
			 */
			if (dss_prev == (void *)-1) {
				/* OOM. */
				atomic_store_b(&dss_exhausted, true,
				    ATOMIC_RELEASE);
				goto label_oom;
			}
		}


// Source: extent_dss.c
// Lines 110-222
