static void empty_workdir(const char *name)
{
	git_vector contents = GIT_VECTOR_INIT;
	char *basename;
	int cmp;
	size_t i;
	const char *fn;

	cl_git_pass(git_fs_path_dirload(&contents, name, 0, 0));
	git_vector_foreach(&contents, i, fn) {
		cl_assert(basename = git_fs_path_basename(fn));
		cmp = strncasecmp(basename, ".git", 4);

		git__free(basename);

		if (cmp)
			cl_git_pass(p_unlink(fn));
	}
	git_vector_free_deep(&contents);
}


// Source: crlf.c
// Lines 182-201
