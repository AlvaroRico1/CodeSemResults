static int apply_patchfile(
	const char *old,
	size_t old_len,
	const char *new,
	size_t new_len,
	const char *patchfile,
	const char *filename_expected,
	unsigned int mode_expected)
{
	git_patch *patch;
	git_str result = GIT_STR_INIT;
	git_str patchbuf = GIT_STR_INIT;
	char *filename;
	unsigned int mode;
	int error;

	cl_git_pass(git_patch_from_buffer(&patch, patchfile, strlen(patchfile), NULL));

	error = git_apply__patch(&result, &filename, &mode, old, old_len, patch, NULL);

	if (error == 0) {
		cl_assert_equal_i(new_len, result.size);
		if (new_len)
			cl_assert(memcmp(new, result.ptr, new_len) == 0);

		cl_assert_equal_s(filename_expected, filename);
		cl_assert_equal_i(mode_expected, mode);
	}

	git__free(filename);
	git_str_dispose(&result);
	git_str_dispose(&patchbuf);
	git_patch_free(patch);

	return error;
}


// Source: fromfile.c
// Lines 23-58
