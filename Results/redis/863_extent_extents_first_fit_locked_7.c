extents_first_fit_locked(tsdn_t *tsdn, arena_t *arena, extents_t *extents,
    size_t size) {
	extent_t *ret = NULL;

	pszind_t pind = sz_psz2ind(extent_size_quantize_ceil(size));
	for (pszind_t i = (pszind_t)bitmap_ffu(extents->bitmap,
	    &extents_bitmap_info, (size_t)pind); i < NPSIZES+1; i =
	    (pszind_t)bitmap_ffu(extents->bitmap, &extents_bitmap_info,
	    (size_t)i+1)) {
		assert(!extent_heap_empty(&extents->heaps[i]));
		extent_t *extent = extent_heap_first(&extents->heaps[i]);
		assert(extent_size_get(extent) >= size);
		if (ret == NULL || extent_snad_comp(extent, ret) < 0) {
			ret = extent;
		}
		if (i == NPSIZES) {
			break;
		}
		assert(i < NPSIZES);
	}


// Source: extent.c
// Lines 426-445
