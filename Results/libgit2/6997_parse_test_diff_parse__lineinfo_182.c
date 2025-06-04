void test_diff_parse__lineinfo(void)
{
	const char *text = PATCH_ORIGINAL_TO_CHANGE_MIDDLE;
	git_diff *diff;
	git_patch *patch;
	const git_diff_hunk *hunk;
	size_t n, l = 0;

	cl_git_pass(git_diff_from_buffer(&diff, text, strlen(text)));
	cl_git_pass(git_patch_from_diff(&patch, diff, 0));
	cl_git_pass(git_patch_get_hunk(&hunk, &n, patch, 0));

	cl_git_assert_lineinfo(3, 3, 1, patch, 0, l++);
	cl_git_assert_lineinfo(4, 4, 1, patch, 0, l++);
	cl_git_assert_lineinfo(5, 5, 1, patch, 0, l++);
	cl_git_assert_lineinfo(6, -1, 1, patch, 0, l++);
	cl_git_assert_lineinfo(-1, 6, 1, patch, 0, l++);
	cl_git_assert_lineinfo(7, 7, 1, patch, 0, l++);
	cl_git_assert_lineinfo(8, 8, 1, patch, 0, l++);
	cl_git_assert_lineinfo(9, 9, 1, patch, 0, l++);

	cl_assert_equal_i(n, l);

	git_patch_free(patch);
	git_diff_free(diff);
}


// Source: parse.c
// Lines 388-413
