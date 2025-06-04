static unsigned long get_size_by_pos(struct bitmap_index *bitmap_git,
				     uint32_t pos)
{
	unsigned long size;
	struct object_info oi = OBJECT_INFO_INIT;

	oi.sizep = &size;

	if (pos < bitmap_num_objects(bitmap_git)) {
		struct packed_git *pack;
		off_t ofs;

		if (bitmap_is_midx(bitmap_git)) {
			uint32_t midx_pos = pack_pos_to_midx(bitmap_git->midx, pos);
			uint32_t pack_id = nth_midxed_pack_int_id(bitmap_git->midx, midx_pos);

			pack = bitmap_git->midx->packs[pack_id];
			ofs = nth_midxed_offset(bitmap_git->midx, midx_pos);
		} else {
			pack = bitmap_git->pack;
			ofs = pack_pos_to_offset(pack, pos);
		}

		if (packed_object_info(the_repository, pack, ofs, &oi) < 0) {
			struct object_id oid;
			nth_bitmap_object_oid(bitmap_git, &oid,
					      pack_pos_to_index(pack, pos));
			die(_("unable to get size of %s"), oid_to_hex(&oid));
		}
	} else {
		struct eindex *eindex = &bitmap_git->ext_index;
		struct object *obj = eindex->objects[pos - bitmap_num_objects(bitmap_git)];
		if (oid_object_info_extended(the_repository, &obj->oid, &oi, 0) < 0)
			die(_("unable to get size of %s"), oid_to_hex(&obj->oid));
	}

	return size;
}


// Source: pack-bitmap.c
// Lines 1037-1074
