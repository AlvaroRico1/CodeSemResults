static int validate_and_apply_patchfile(
	const char *old,
	size_t old_len,
	const char *new,
	size_t new_len,
	const char *patchfile,
	const git_diff_options *diff_opts,
	const char *filename_expected,
	unsigned int mode_expected)
{
	git_patch *patch_fromdiff;
	git_buf validated = GIT_BUF_INIT;
	int error;

	cl_git_pass(git_patch_from_buffers(&patch_fromdiff,
		old, old_len, "file.txt",
		new, new_len, "file.txt",
		diff_opts));
	cl_git_pass(git_patch_to_buf(&validated, patch_fromdiff));

	cl_assert_equal_s(patchfile, validated.ptr);

	error = apply_patchfile(old, old_len, new, new_len, patchfile, filename_expected, mode_expected);

	git_buf_dispose(&validated);
	git_patch_free(patch_fromdiff);

	return error;
}


// Source: fromfile.c
// Lines 60-88
