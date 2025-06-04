static int index_entry_similarity_calc(
	void **out,
	git_repository *repo,
	git_index_entry *entry,
	const git_merge_options *opts)
{
	git_blob *blob;
	git_diff_file diff_file = {{{0}}};
	git_object_size_t blobsize;
	int error;

	if (*out || *out == &cache_invalid_marker)
		return 0;

	*out = NULL;

	if ((error = git_blob_lookup(&blob, repo, &entry->id)) < 0)
		return error;

	git_oid_cpy(&diff_file.id, &entry->id);
	diff_file.path = entry->path;
	diff_file.size = entry->file_size;
	diff_file.mode = entry->mode;
	diff_file.flags = 0;

	blobsize = git_blob_rawsize(blob);

	/* file too big for rename processing */
	if (!git__is_sizet(blobsize))
		return 0;

	error = opts->metric->buffer_signature(out, &diff_file,
		git_blob_rawcontent(blob), (size_t)blobsize,
		opts->metric->payload);
	if (error == GIT_EBUFS)
		*out = &cache_invalid_marker;

	git_blob_free(blob);

	return error;
}


// Source: merge.c
// Lines 1057-1097
