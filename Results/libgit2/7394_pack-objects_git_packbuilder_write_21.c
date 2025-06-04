int git_packbuilder_write(
	git_packbuilder *pb,
	const char *path,
	unsigned int mode,
	git_indexer_progress_cb progress_cb,
	void *progress_cb_payload)
{
	int error = -1;
	git_str object_path = GIT_STR_INIT;
	git_indexer_options opts = GIT_INDEXER_OPTIONS_INIT;
	git_indexer *indexer = NULL;
	git_indexer_progress stats;
	struct pack_write_context ctx;
	int t;

	PREPARE_PACK;

	if (path == NULL) {
		if ((error = git_repository__item_path(&object_path, pb->repo, GIT_REPOSITORY_ITEM_OBJECTS)) < 0)
			goto cleanup;
		if ((error = git_str_joinpath(&object_path, git_str_cstr(&object_path), "pack")) < 0)
			goto cleanup;
		path = git_str_cstr(&object_path);
	}

	opts.progress_cb = progress_cb;
	opts.progress_cb_payload = progress_cb_payload;

	if ((error = git_indexer_new(&indexer, path, mode, pb->odb, &opts)) < 0)
		goto cleanup;

	if (!git_repository__configmap_lookup(&t, pb->repo, GIT_CONFIGMAP_FSYNCOBJECTFILES) && t)
		git_indexer__set_fsync(indexer, 1);

	ctx.indexer = indexer;
	ctx.stats = &stats;

	if ((error = git_packbuilder_foreach(pb, write_cb, &ctx)) < 0)
		goto cleanup;

	if ((error = git_indexer_commit(indexer, &stats)) < 0)
		goto cleanup;

#ifndef GIT_DEPRECATE_HARD
	git_oid_cpy(&pb->pack_oid, git_indexer_hash(indexer));
#endif

	pb->pack_name = git__strdup(git_indexer_name(indexer));
	GIT_ERROR_CHECK_ALLOC(pb->pack_name);

cleanup:
	git_indexer_free(indexer);
	git_str_dispose(&object_path);
	return error;
}


// Source: pack-objects.c
// Lines 1382-1436
