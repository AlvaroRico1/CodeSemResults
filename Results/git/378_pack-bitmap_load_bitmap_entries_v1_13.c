static int load_bitmap_entries_v1(struct bitmap_index *index)
{
	uint32_t i;
	struct stored_bitmap *recent_bitmaps[MAX_XOR_OFFSET] = { NULL };

	for (i = 0; i < index->entry_count; ++i) {
		int xor_offset, flags;
		struct ewah_bitmap *bitmap = NULL;
		struct stored_bitmap *xor_bitmap = NULL;
		uint32_t commit_idx_pos;
		struct object_id oid;

		if (index->map_size - index->map_pos < 6)
			return error("corrupt ewah bitmap: truncated header for entry %d", i);

		commit_idx_pos = read_be32(index->map, &index->map_pos);
		xor_offset = read_u8(index->map, &index->map_pos);
		flags = read_u8(index->map, &index->map_pos);

		if (nth_bitmap_object_oid(index, &oid, commit_idx_pos) < 0)
			return error("corrupt ewah bitmap: commit index %u out of range",
				     (unsigned)commit_idx_pos);

		bitmap = read_bitmap_1(index);
		if (!bitmap)
			return -1;

		if (xor_offset > MAX_XOR_OFFSET || xor_offset > i)
			return error("Corrupted bitmap pack index");

		if (xor_offset > 0) {
			xor_bitmap = recent_bitmaps[(i - xor_offset) % MAX_XOR_OFFSET];

			if (xor_bitmap == NULL)
				return error("Invalid XOR offset in bitmap pack index");
		}

		recent_bitmaps[i % MAX_XOR_OFFSET] = store_bitmap(
			index, bitmap, &oid, xor_bitmap, flags);
	}


// Source: pack-bitmap.c
// Lines 249-288
