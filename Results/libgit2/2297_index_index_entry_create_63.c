static int index_entry_create(
	git_index_entry **out,
	git_repository *repo,
	const char *path,
	struct stat *st,
	bool from_workdir)
{
	size_t pathlen = strlen(path), alloclen;
	struct entry_internal *entry;
	unsigned int path_valid_flags = GIT_PATH_REJECT_INDEX_DEFAULTS;
	uint16_t mode = 0;

	/* always reject placing `.git` in the index and directory traversal.
	 * when requested, disallow platform-specific filenames and upgrade to
	 * the platform-specific `.git` tests (eg, `git~1`, etc).
	 */
	if (from_workdir)
		path_valid_flags |= GIT_PATH_REJECT_WORKDIR_DEFAULTS;
	if (st)
		mode = st->st_mode;

	if (!git_path_is_valid(repo, path, mode, path_valid_flags)) {
		git_error_set(GIT_ERROR_INDEX, "invalid path: '%s'", path);
		return -1;
	}

	GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, sizeof(struct entry_internal), pathlen);
	GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, alloclen, 1);
	entry = git__calloc(1, alloclen);
	GIT_ERROR_CHECK_ALLOC(entry);

	entry->pathlen = pathlen;
	memcpy(entry->path, path, pathlen);
	entry->entry.path = entry->path;

	*out = (git_index_entry *)entry;
	return 0;
}


// Source: index.c
// Lines 919-956
