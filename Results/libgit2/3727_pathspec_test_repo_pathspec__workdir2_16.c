void test_repo_pathspec__workdir2(void)
{
	git_strarray s;
	git_pathspec *ps;
	git_pathspec_match_list *m;

	/* { "staged_*" } */
	s.strings = str2; s.count = ARRAY_SIZE(str2);
	cl_git_pass(git_pathspec_new(&ps, &s));

	cl_git_pass(git_pathspec_match_workdir(&m, g_repo, 0, ps));
	cl_assert_equal_sz(5, git_pathspec_match_list_entrycount(m));
	git_pathspec_match_list_free(m);

	cl_git_pass(git_pathspec_match_workdir(&m, g_repo,
		GIT_PATHSPEC_FIND_FAILURES, ps));
	cl_assert_equal_sz(5, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_sz(0, git_pathspec_match_list_failed_entrycount(m));
	git_pathspec_match_list_free(m);

	cl_git_fail(git_pathspec_match_workdir(&m, g_repo,
		GIT_PATHSPEC_NO_GLOB | GIT_PATHSPEC_NO_MATCH_ERROR, ps));

	cl_git_pass(git_pathspec_match_workdir(&m, g_repo,
		GIT_PATHSPEC_NO_GLOB | GIT_PATHSPEC_FIND_FAILURES, ps));
	cl_assert_equal_sz(0, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_sz(1, git_pathspec_match_list_failed_entrycount(m));
	git_pathspec_match_list_free(m);

	git_pathspec_free(ps);
}


// Source: pathspec.c
// Lines 93-123
