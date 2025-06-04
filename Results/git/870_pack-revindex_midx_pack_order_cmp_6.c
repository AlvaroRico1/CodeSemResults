static int midx_pack_order_cmp(const void *va, const void *vb)
{
	const struct midx_pack_key *key = va;
	struct multi_pack_index *midx = key->midx;

	uint32_t versus = pack_pos_to_midx(midx, (uint32_t*)vb - (const uint32_t *)midx->revindex_data);
	uint32_t versus_pack = nth_midxed_pack_int_id(midx, versus);
	off_t versus_offset;

	uint32_t key_preferred = key->pack == key->preferred_pack;
	uint32_t versus_preferred = versus_pack == key->preferred_pack;

	/*
	 * First, compare the preferred-ness, noting that the preferred pack
	 * comes first.
	 */
	if (key_preferred && !versus_preferred)
		return -1;
	else if (!key_preferred && versus_preferred)
		return 1;

	/* Then, break ties first by comparing the pack IDs. */
	if (key->pack < versus_pack)
		return -1;
	else if (key->pack > versus_pack)
		return 1;

	/* Finally, break ties by comparing offsets within a pack. */
	versus_offset = nth_midxed_offset(midx, versus);
	if (key->offset < versus_offset)
		return -1;
	else if (key->offset > versus_offset)
		return 1;

	return 0;
}


// Source: pack-revindex.c
// Lines 406-441
