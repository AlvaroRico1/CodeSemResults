static int fill_included_packs_batch(struct repository *r,
				     struct multi_pack_index *m,
				     unsigned char *include_pack,
				     size_t batch_size)
{
	uint32_t i, packs_to_repack;
	size_t total_size;
	struct repack_info *pack_info = xcalloc(m->num_packs, sizeof(struct repack_info));
	int pack_kept_objects = 0;

	repo_config_get_bool(r, "repack.packkeptobjects", &pack_kept_objects);

	for (i = 0; i < m->num_packs; i++) {
		pack_info[i].pack_int_id = i;

		if (prepare_midx_pack(r, m, i))
			continue;

		pack_info[i].mtime = m->packs[i]->mtime;
	}

	for (i = 0; batch_size && i < m->num_objects; i++) {
		uint32_t pack_int_id = nth_midxed_pack_int_id(m, i);
		pack_info[pack_int_id].referenced_objects++;
	}

	QSORT(pack_info, m->num_packs, compare_by_mtime);

	total_size = 0;
	packs_to_repack = 0;
	for (i = 0; total_size < batch_size && i < m->num_packs; i++) {
		int pack_int_id = pack_info[i].pack_int_id;
		struct packed_git *p = m->packs[pack_int_id];
		size_t expected_size;

		if (!p)
			continue;
		if (!pack_kept_objects && p->pack_keep)
			continue;
		if (open_pack_index(p) || !p->num_objects)
			continue;

		expected_size = (size_t)(p->pack_size
					 * pack_info[i].referenced_objects);
		expected_size /= p->num_objects;

		if (expected_size >= batch_size)
			continue;

		packs_to_repack++;
		total_size += expected_size;
		include_pack[pack_int_id] = 1;
	}

	free(pack_info);

	if (packs_to_repack < 2)
		return 1;

	return 0;
}


// Source: midx.c
// Lines 1702-1762
