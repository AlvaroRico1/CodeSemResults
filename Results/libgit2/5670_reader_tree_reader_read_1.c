static int tree_reader_read(
	git_str *out,
	git_oid *out_id,
	git_filemode_t *out_filemode,
	git_reader *_reader,
	const char *filename)
{
	tree_reader *reader = (tree_reader *)_reader;
	git_tree_entry *tree_entry = NULL;
	git_blob *blob = NULL;
	git_object_size_t blobsize;
	int error;

	if ((error = git_tree_entry_bypath(&tree_entry, reader->tree, filename)) < 0 ||
	    (error = git_blob_lookup(&blob, git_tree_owner(reader->tree), git_tree_entry_id(tree_entry))) < 0)
		goto done;

	blobsize = git_blob_rawsize(blob);
	GIT_ERROR_CHECK_BLOBSIZE(blobsize);

	if ((error = git_str_set(out, git_blob_rawcontent(blob), (size_t)blobsize)) < 0)
		goto done;

	if (out_id)
		git_oid_cpy(out_id, git_tree_entry_id(tree_entry));

	if (out_filemode)
		*out_filemode = git_tree_entry_filemode(tree_entry);

done:
	git_blob_free(blob);
	git_tree_entry_free(tree_entry);
	return error;
}


// Source: reader.c
// Lines 25-58
