static int add_object_entry(const struct object_id *oid, enum object_type type,
			    const char *name, int exclude)
{
	struct packed_git *found_pack = NULL;
	off_t found_offset = 0;

	display_progress(progress_state, ++nr_seen);

	if (have_duplicate_entry(oid, exclude))
		return 0;

	if (!want_object_in_pack(oid, exclude, &found_pack, &found_offset)) {
		/* The pack is missing an object, so it will not have closure */
		if (write_bitmap_index) {
			if (write_bitmap_index != WRITE_BITMAP_QUIET)
				warning(_(no_closure_warning));
			write_bitmap_index = 0;
		}
		return 0;
	}

	create_object_entry(oid, type, pack_name_hash(name),
			    exclude, name && no_try_delta(name),
			    found_pack, found_offset);
	return 1;
}


// Source: pack-objects.c
// Lines 1539-1564
