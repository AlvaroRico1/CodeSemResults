void test_diff_diffiter__iterate_files_2(void)
{
	git_repository *repo = cl_git_sandbox_init("status");
	git_diff *diff;
	size_t d, num_d;
	int count = 0;

	cl_git_pass(git_diff_index_to_workdir(&diff, repo, NULL, NULL));

	num_d = git_diff_num_deltas(diff);
	cl_assert_equal_i(8, (int)num_d);

	for (d = 0; d < num_d; ++d) {
		const git_diff_delta *delta = git_diff_get_delta(diff, d);
		cl_assert(delta != NULL);
		count++;
	}


// Source: diffiter.c
// Lines 54-70
