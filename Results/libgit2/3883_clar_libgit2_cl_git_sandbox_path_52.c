const char *cl_git_sandbox_path(int is_dir, ...)
{
	const char *path = NULL;
	static char _temp[GIT_PATH_MAX];
	git_str buf = GIT_STR_INIT;
	va_list arg;

	cl_git_pass(git_str_sets(&buf, clar_sandbox_path()));

	va_start(arg, is_dir);

	while ((path = va_arg(arg, const char *)) != NULL) {
		cl_git_pass(git_str_joinpath(&buf, buf.ptr, path));
	}
	va_end(arg);

	cl_git_pass(git_fs_path_prettify(&buf, buf.ptr, NULL));
	if (is_dir)
		git_fs_path_to_dir(&buf);

	/* make sure we won't truncate */
	cl_assert(git_str_len(&buf) < sizeof(_temp));
	git_str_copy_cstr(_temp, sizeof(_temp), &buf);

	git_str_dispose(&buf);

	return _temp;
}


// Source: clar_libgit2.c
// Lines 323-350
