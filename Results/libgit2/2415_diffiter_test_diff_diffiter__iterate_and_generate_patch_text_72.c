void test_diff_diffiter__iterate_and_generate_patch_text(void)
{
	git_repository *repo = cl_git_sandbox_init("status");
	git_diff *diff;
	size_t d, num_d;

	cl_git_pass(git_diff_index_to_workdir(&diff, repo, NULL, NULL));

	num_d = git_diff_num_deltas(diff);
	cl_assert_equal_i(8, (int)num_d);

	for (d = 0; d < num_d; ++d) {
		git_patch *patch;
		git_buf buf = GIT_BUF_INIT;

		cl_git_pass(git_patch_from_diff(&patch, diff, d));
		cl_assert(patch != NULL);

		cl_git_pass(git_patch_to_buf(&buf, patch));

		cl_assert_equal_s(expected_patch_text[d], buf.ptr);

		git_buf_dispose(&buf);
		git_patch_free(patch);
	}


// Source: diffiter.c
// Lines 404-428
