int cl_git_remove_placeholders(const char *directory_path, const char *filename)
{
	int error;
	remove_data data;
	git_str buffer = GIT_STR_INIT;

	if (git_fs_path_isdir(directory_path) == false)
		return -1;

	if (git_str_sets(&buffer, directory_path) < 0)
		return -1;

	data.filename = filename;
	data.filename_len = strlen(filename);

	error = remove_placeholders_recurs(&data, &buffer);

	git_str_dispose(&buffer);

	return error;
}


// Source: clar_libgit2.c
// Lines 379-399
