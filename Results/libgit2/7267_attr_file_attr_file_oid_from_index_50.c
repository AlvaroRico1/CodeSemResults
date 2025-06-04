static int attr_file_oid_from_index(
	git_oid *oid, git_repository *repo, const char *path)
{
	int error;
	git_index *idx;
	size_t pos;
	const git_index_entry *entry;

	if ((error = git_repository_index__weakptr(&idx, repo)) < 0 ||
		(error = git_index__find_pos(&pos, idx, path, 0, 0)) < 0)
		return error;

	if (!(entry = git_index_get_byindex(idx, pos)))
		return GIT_ENOTFOUND;

	*oid = entry->id;
	return 0;
}


// Source: attr_file.c
// Lines 87-104
