static int open_pack_bitmap(struct repository *r,
			    struct bitmap_index *bitmap_git)
{
	struct packed_git *p;
	int ret = -1;

	assert(!bitmap_git->map);

	for (p = get_all_packs(r); p; p = p->next) {
		if (open_pack_bitmap_1(bitmap_git, p) == 0)
			ret = 0;
	}

	return ret;
}


// Source: pack-bitmap.c
// Lines 469-483
