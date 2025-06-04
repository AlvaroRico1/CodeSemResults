static uint32_t midx_preferred_pack(struct bitmap_index *bitmap_git)
{
	struct multi_pack_index *m = bitmap_git->midx;
	if (!m)
		BUG("midx_preferred_pack: requires non-empty MIDX");
	return nth_midxed_pack_int_id(m, pack_pos_to_midx(bitmap_git->midx, 0));
}


// Source: pack-bitmap.c
// Lines 1421-1427
