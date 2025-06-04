int git_fs_path_dirload(
	git_vector *contents,
	const char *path,
	size_t prefix_len,
	uint32_t flags)
{
	git_fs_path_diriter iter = GIT_FS_PATH_DIRITER_INIT;
	const char *name;
	size_t name_len;
	char *dup;
	int error;

	GIT_ASSERT_ARG(contents);
	GIT_ASSERT_ARG(path);

	if ((error = git_fs_path_diriter_init(&iter, path, flags)) < 0)
		return error;

	while ((error = git_fs_path_diriter_next(&iter)) == 0) {
		if ((error = git_fs_path_diriter_fullpath(&name, &name_len, &iter)) < 0)
			break;

		GIT_ASSERT(name_len > prefix_len);

		dup = git__strndup(name + prefix_len, name_len - prefix_len);
		GIT_ERROR_CHECK_ALLOC(dup);

		if ((error = git_vector_insert(contents, dup)) < 0)
			break;
	}

	if (error == GIT_ITEROVER)
		error = 0;

	git_fs_path_diriter_free(&iter);
	return error;
}


// Source: fs_path.c
// Lines 1501-1537
