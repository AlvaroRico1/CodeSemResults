void test_diff_diffiter__iterate_files_1(void)
{
	git_repository *repo = cl_git_sandbox_init("attr");
	git_diff *diff;
	size_t d, num_d;
	diff_expects exp = { 0 };

	cl_git_pass(git_diff_index_to_workdir(&diff, repo, NULL, NULL));

	num_d = git_diff_num_deltas(diff);

	for (d = 0; d < num_d; ++d) {
		const git_diff_delta *delta = git_diff_get_delta(diff, d);
		cl_assert(delta != NULL);

		diff_file_cb(delta, (float)d / (float)num_d, &exp);
	}


// Source: diffiter.c
// Lines 32-48
