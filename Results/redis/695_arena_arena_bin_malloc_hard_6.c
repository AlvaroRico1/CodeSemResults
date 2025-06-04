arena_bin_malloc_hard(tsdn_t *tsdn, arena_t *arena, bin_t *bin,
    szind_t binind) {
	const bin_info_t *bin_info;
	extent_t *slab;

	bin_info = &bin_infos[binind];
	if (!arena_is_auto(arena) && bin->slabcur != NULL) {
		arena_bin_slabs_full_insert(arena, bin, bin->slabcur);
		bin->slabcur = NULL;
	}
	slab = arena_bin_nonfull_slab_get(tsdn, arena, bin, binind);
	if (bin->slabcur != NULL) {
		/*
		 * Another thread updated slabcur while this one ran without the
		 * bin lock in arena_bin_nonfull_slab_get().
		 */
		if (extent_nfree_get(bin->slabcur) > 0) {
			void *ret = arena_slab_reg_alloc(bin->slabcur,
			    bin_info);
			if (slab != NULL) {
				/*
				 * arena_slab_alloc() may have allocated slab,
				 * or it may have been pulled from
				 * slabs_nonfull.  Therefore it is unsafe to
				 * make any assumptions about how slab has
				 * previously been used, and
				 * arena_bin_lower_slab() must be called, as if
				 * a region were just deallocated from the slab.
				 */
				if (extent_nfree_get(slab) == bin_info->nregs) {
					arena_dalloc_bin_slab(tsdn, arena, slab,
					    bin);
				} else {
					arena_bin_lower_slab(tsdn, arena, slab,
					    bin);
				}
			}
			return ret;
		}

		arena_bin_slabs_full_insert(arena, bin, bin->slabcur);
		bin->slabcur = NULL;
	}

	if (slab == NULL) {
		return NULL;
	}
	bin->slabcur = slab;

	assert(extent_nfree_get(bin->slabcur) > 0);

	return arena_slab_reg_alloc(slab, bin_info);
}


// Source: arena.c
// Lines 1195-1247
