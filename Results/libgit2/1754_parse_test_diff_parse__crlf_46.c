void test_diff_parse__crlf(void)
{
	const char *text = PATCH_CRLF;
	git_diff *diff;
	git_patch *patch;
	const git_diff_delta *delta;

	cl_git_pass(git_diff_from_buffer(&diff, text, strlen(text)));
	cl_git_pass(git_patch_from_diff(&patch, diff, 0));
	delta = git_patch_get_delta(patch);

	cl_assert_equal_s(delta->old_file.path, "test-file");
	cl_assert_equal_s(delta->new_file.path, "test-file");

	git_patch_free(patch);
	git_diff_free(diff);
}


// Source: parse.c
// Lines 434-450
