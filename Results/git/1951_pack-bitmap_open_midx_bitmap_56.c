static int open_midx_bitmap(struct repository *r,
			    struct bitmap_index *bitmap_git)
{
	struct multi_pack_index *midx;

	assert(!bitmap_git->map);

	for (midx = get_multi_pack_index(r); midx; midx = midx->next) {
		if (!open_midx_bitmap_1(bitmap_git, midx))
			return 0;
	}
	return -1;
}


// Source: pack-bitmap.c
// Lines 485-497
