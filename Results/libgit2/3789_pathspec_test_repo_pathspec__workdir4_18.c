void test_repo_pathspec__workdir4(void)
{
	git_strarray s;
	git_pathspec *ps;
	git_pathspec_match_list *m;

	/* { "*" } */
	s.strings = str4; s.count = ARRAY_SIZE(str4);
	cl_git_pass(git_pathspec_new(&ps, &s));

	cl_git_pass(git_pathspec_match_workdir(&m, g_repo, 0, ps));
	cl_assert_equal_sz(13, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_s("\xE8\xBF\x99", git_pathspec_match_list_entry(m, 12));
	git_pathspec_match_list_free(m);

	git_pathspec_free(ps);
}


// Source: pathspec.c
// Lines 158-174
