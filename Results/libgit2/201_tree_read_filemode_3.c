static mode_t read_filemode(const char *path)
{
	git_str fullpath = GIT_STR_INIT;
	struct stat st;
	mode_t result;

	git_str_joinpath(&fullpath, "testrepo", path);
	cl_must_pass(p_stat(fullpath.ptr, &st));

	result = GIT_PERMS_IS_EXEC(st.st_mode) ?
		GIT_FILEMODE_BLOB_EXECUTABLE : GIT_FILEMODE_BLOB;

	git_str_dispose(&fullpath);

	return result;
}


// Source: tree.c
// Lines 1038-1053
