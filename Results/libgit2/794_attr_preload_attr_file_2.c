GIT_INLINE(int) preload_attr_file(
	git_repository *repo,
	git_attr_session *attr_session,
	const char *base,
	const char *filename)
{
	git_attr_file_source source = { GIT_ATTR_FILE_SOURCE_FILE };

	if (!filename)
		return 0;

	source.base = base;
	source.filename = filename;

	return preload_attr_source(repo, attr_session, &source);
}


// Source: attr.c
// Lines 323-338
