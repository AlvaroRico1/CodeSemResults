void test_diff_racediffiter__basic(void)
{
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_repository *repo = cl_git_sandbox_init("diff");
	git_diff *diff;

	basic_payload exp_a[] = {
		{ "another.txt", GIT_DELTA_MODIFIED },
		{ "readme.txt", GIT_DELTA_MODIFIED },
		{ "zzzzz/", GIT_DELTA_IGNORED },
		{ NULL, 0 }
	};

	cl_must_pass(p_mkdir("diff/zzzzz", 0777));

	opts.flags |= GIT_DIFF_INCLUDE_IGNORED | GIT_DIFF_RECURSE_UNTRACKED_DIRS;
	opts.notify_cb = notify_cb__basic;
	opts.payload = exp_a;

	cl_git_pass(git_diff_index_to_workdir(&diff, repo, NULL, &opts));

	git_diff_free(diff);
}


// Source: racediffiter.c
// Lines 59-81
