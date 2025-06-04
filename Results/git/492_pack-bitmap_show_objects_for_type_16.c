static void show_objects_for_type(
	struct bitmap_index *bitmap_git,
	enum object_type object_type,
	show_reachable_fn show_reach)
{
	size_t i = 0;
	uint32_t offset;

	struct ewah_iterator it;
	eword_t filter;

	struct bitmap *objects = bitmap_git->result;

	init_type_iterator(&it, bitmap_git, object_type);

	for (i = 0; i < objects->word_alloc &&
			ewah_iterator_next(&filter, &it); i++) {
		eword_t word = objects->words[i] & filter;
		size_t pos = (i * BITS_IN_EWORD);

		if (!word)
			continue;

		for (offset = 0; offset < BITS_IN_EWORD; ++offset) {
			struct packed_git *pack;
			struct object_id oid;
			uint32_t hash = 0, index_pos;
			off_t ofs;

			if ((word >> offset) == 0)
				break;

			offset += ewah_bit_ctz64(word >> offset);

			if (bitmap_is_midx(bitmap_git)) {
				struct multi_pack_index *m = bitmap_git->midx;
				uint32_t pack_id;

				index_pos = pack_pos_to_midx(m, pos + offset);
				ofs = nth_midxed_offset(m, index_pos);
				nth_midxed_object_oid(&oid, m, index_pos);

				pack_id = nth_midxed_pack_int_id(m, index_pos);
				pack = bitmap_git->midx->packs[pack_id];
			} else {
				index_pos = pack_pos_to_index(bitmap_git->pack, pos + offset);
				ofs = pack_pos_to_offset(bitmap_git->pack, pos + offset);
				nth_bitmap_object_oid(bitmap_git, &oid, index_pos);

				pack = bitmap_git->pack;
			}


// Source: pack-bitmap.c
// Lines 880-930
