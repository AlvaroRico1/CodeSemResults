static int has_cr_in_index(const git_filter_source *src)
{
	git_repository *repo = git_filter_source_repo(src);
	const char *path = git_filter_source_path(src);
	git_index *index;
	const git_index_entry *entry;
	git_blob *blob;
	const void *blobcontent;
	git_object_size_t blobsize;
	bool found_cr;

	if (!path)
		return false;

	if (git_repository_index__weakptr(&index, repo) < 0) {
		git_error_clear();
		return false;
	}

	if (!(entry = git_index_get_bypath(index, path, 0)) &&
		!(entry = git_index_get_bypath(index, path, 1)))
		return false;

	if (!S_ISREG(entry->mode)) /* don't crlf filter non-blobs */
		return true;

	if (git_blob_lookup(&blob, repo, &entry->id) < 0)
		return false;

	blobcontent = git_blob_rawcontent(blob);
	blobsize    = git_blob_rawsize(blob);
	if (!git__is_sizet(blobsize))
		blobsize = (size_t)-1;

	found_cr = (blobcontent != NULL &&
		blobsize > 0 &&
		memchr(blobcontent, '\r', (size_t)blobsize) != NULL);

	git_blob_free(blob);
	return found_cr;
}


// Source: crlf.c
// Lines 73-113
