void test_diff_diffiter__create(void)
{
	git_repository *repo = cl_git_sandbox_init("attr");
	git_diff *diff;
	size_t d, num_d;

	cl_git_pass(git_diff_index_to_workdir(&diff, repo, NULL, NULL));

	num_d = git_diff_num_deltas(diff);
	for (d = 0; d < num_d; ++d) {
		const git_diff_delta *delta = git_diff_get_delta(diff, d);
		cl_assert(delta != NULL);
	}

	cl_assert(!git_diff_get_delta(diff, num_d));

	git_diff_free(diff);
}


// Source: diffiter.c
// Lines 13-30
