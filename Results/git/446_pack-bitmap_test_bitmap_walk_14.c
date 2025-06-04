void test_bitmap_walk(struct rev_info *revs)
{
	struct object *root;
	struct bitmap *result = NULL;
	size_t result_popcnt;
	struct bitmap_test_data tdata;
	struct bitmap_index *bitmap_git;
	struct ewah_bitmap *bm;

	if (!(bitmap_git = prepare_bitmap_git(revs->repo)))
		die("failed to load bitmap indexes");

	if (revs->pending.nr != 1)
		die("you must specify exactly one commit to test");

	fprintf(stderr, "Bitmap v%d test (%d entries loaded)\n",
		bitmap_git->version, bitmap_git->entry_count);

	root = revs->pending.objects[0].item;
	bm = bitmap_for_commit(bitmap_git, (struct commit *)root);

	if (bm) {
		fprintf(stderr, "Found bitmap for %s. %d bits / %08x checksum\n",
			oid_to_hex(&root->oid), (int)bm->bit_size, ewah_checksum(bm));

		result = ewah_to_bitmap(bm);
	}

	if (result == NULL)
		die("Commit %s doesn't have an indexed bitmap", oid_to_hex(&root->oid));

	revs->tag_objects = 1;
	revs->tree_objects = 1;
	revs->blob_objects = 1;

	result_popcnt = bitmap_popcount(result);

	if (prepare_revision_walk(revs))
		die("revision walk setup failed");

	tdata.bitmap_git = bitmap_git;
	tdata.base = bitmap_new();
	tdata.commits = ewah_to_bitmap(bitmap_git->commits);
	tdata.trees = ewah_to_bitmap(bitmap_git->trees);
	tdata.blobs = ewah_to_bitmap(bitmap_git->blobs);
	tdata.tags = ewah_to_bitmap(bitmap_git->tags);
	tdata.prg = start_progress("Verifying bitmap entries", result_popcnt);
	tdata.seen = 0;

	traverse_commit_list(revs, &test_show_commit, &test_show_object, &tdata);

	stop_progress(&tdata.prg);

	if (bitmap_equals(result, tdata.base))
		fprintf(stderr, "OK!\n");
	else
		die("mismatch in bitmap results");

	free_bitmap_index(bitmap_git);
}


// Source: pack-bitmap.c
// Lines 1666-1725
