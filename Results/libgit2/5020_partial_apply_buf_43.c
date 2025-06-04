static int apply_buf(
	const char *old,
	const char *oldname,
	const char *new,
	const char *newname,
	const char *expected,
	const git_diff_options *diff_opts,
	git_apply_hunk_cb hunk_cb,
	void *payload)
{
	git_patch *patch;
	git_str result = GIT_STR_INIT;
	git_str patchbuf = GIT_STR_INIT;
	git_apply_options opts = GIT_APPLY_OPTIONS_INIT;
	char *filename;
	unsigned int mode;
	int error;
	size_t oldsize = strlen(old);
	size_t newsize = strlen(new);

	opts.hunk_cb = hunk_cb;
	opts.payload = payload;

	cl_git_pass(git_patch_from_buffers(&patch, old, oldsize, oldname, new, newsize, newname, diff_opts));
	if ((error = git_apply__patch(&result, &filename, &mode, old, oldsize, patch, &opts)) == 0) {
		cl_assert_equal_s(expected, result.ptr);
		cl_assert_equal_s(newname, filename);
		cl_assert_equal_i(0100644, mode);
	}

	git__free(filename);
	git_str_dispose(&result);
	git_str_dispose(&patchbuf);
	git_patch_free(patch);

	return error;
}


// Source: partial.c
// Lines 75-111
