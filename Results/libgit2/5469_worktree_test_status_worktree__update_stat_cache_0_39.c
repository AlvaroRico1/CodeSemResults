void test_status_worktree__update_stat_cache_0(void)
{
	git_repository *repo = cl_git_sandbox_init("status");
	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	git_status_list *status;
	git_diff_perfdata perf = GIT_DIFF_PERFDATA_INIT;
	git_index *index;

	opts.flags = GIT_STATUS_OPT_DEFAULTS;

	cl_git_pass(git_status_list_new(&status, repo, &opts));
	check_status0(status);
	cl_git_pass(git_status_list_get_perfdata(&perf, status));
	cl_assert_equal_sz(13 + 3, perf.stat_calls);
	cl_assert_equal_sz(5, perf.oid_calculations);

	git_status_list_free(status);

	/* tick the index so we avoid recalculating racily-clean entries */
	cl_git_pass(git_repository_index__weakptr(&index, repo));
	tick_index(index);

	opts.flags |= GIT_STATUS_OPT_UPDATE_INDEX;

	cl_git_pass(git_status_list_new(&status, repo, &opts));
	check_status0(status);
	cl_git_pass(git_status_list_get_perfdata(&perf, status));
	cl_assert_equal_sz(13 + 3, perf.stat_calls);
	cl_assert_equal_sz(5, perf.oid_calculations);

	git_status_list_free(status);

	opts.flags &= ~GIT_STATUS_OPT_UPDATE_INDEX;

	/* tick again as the index updating from the previous diff might have reset the timestamp */
	tick_index(index);
	cl_git_pass(git_status_list_new(&status, repo, &opts));
	check_status0(status);
	cl_git_pass(git_status_list_get_perfdata(&perf, status));
	cl_assert_equal_sz(13 + 3, perf.stat_calls);
	cl_assert_equal_sz(0, perf.oid_calculations);

	git_status_list_free(status);
}


// Source: worktree.c
// Lines 1035-1078
