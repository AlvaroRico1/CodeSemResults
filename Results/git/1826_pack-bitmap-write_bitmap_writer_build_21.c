int bitmap_writer_build(struct packing_data *to_pack)
{
	struct bitmap_builder bb;
	size_t i;
	int nr_stored = 0; /* for progress */
	struct prio_queue queue = { compare_commits_by_gen_then_commit_date };
	struct prio_queue tree_queue = { NULL };
	struct bitmap_index *old_bitmap;
	uint32_t *mapping;
	int closed = 1; /* until proven otherwise */

	writer.bitmaps = kh_init_oid_map();
	writer.to_pack = to_pack;

	if (writer.show_progress)
		writer.progress = start_progress("Building bitmaps", writer.selected_nr);
	trace2_region_enter("pack-bitmap-write", "building_bitmaps_total",
			    the_repository);

	old_bitmap = prepare_bitmap_git(to_pack->repo);
	if (old_bitmap)
		mapping = create_bitmap_mapping(old_bitmap, to_pack);
	else
		mapping = NULL;

	bitmap_builder_init(&bb, &writer, old_bitmap);
	for (i = bb.commits_nr; i > 0; i--) {
		struct commit *commit = bb.commits[i-1];
		struct bb_commit *ent = bb_data_at(&bb.data, commit);
		struct commit *child;
		int reused = 0;

		if (fill_bitmap_commit(ent, commit, &queue, &tree_queue,
				       old_bitmap, mapping) < 0) {
			closed = 0;
			break;
		}

		if (ent->selected) {
			store_selected(ent, commit);
			nr_stored++;
			display_progress(writer.progress, nr_stored);
		}

		while ((child = pop_commit(&ent->reverse_edges))) {
			struct bb_commit *child_ent =
				bb_data_at(&bb.data, child);

			if (child_ent->bitmap)
				bitmap_or(child_ent->bitmap, ent->bitmap);
			else if (reused)
				child_ent->bitmap = bitmap_dup(ent->bitmap);
			else {
				child_ent->bitmap = ent->bitmap;
				reused = 1;
			}
		}


// Source: pack-bitmap-write.c
// Lines 459-515
