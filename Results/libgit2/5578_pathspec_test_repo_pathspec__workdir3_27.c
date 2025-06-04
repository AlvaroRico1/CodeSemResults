void test_repo_pathspec__workdir3(void)
{
	git_strarray s;
	git_pathspec *ps;
	git_pathspec_match_list *m;

	/* { "!subdir", "*_file", "new_file" } */
	s.strings = str3; s.count = ARRAY_SIZE(str3);
	cl_git_pass(git_pathspec_new(&ps, &s));

	cl_git_pass(git_pathspec_match_workdir(&m, g_repo, 0, ps));
	cl_assert_equal_sz(7, git_pathspec_match_list_entrycount(m));
	git_pathspec_match_list_free(m);

	cl_git_pass(git_pathspec_match_workdir(&m, g_repo,
		GIT_PATHSPEC_FIND_FAILURES, ps));
	cl_assert_equal_sz(7, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_sz(0, git_pathspec_match_list_failed_entrycount(m));

	cl_assert_equal_s("current_file", git_pathspec_match_list_entry(m, 0));
	cl_assert_equal_s("modified_file", git_pathspec_match_list_entry(m, 1));
	cl_assert_equal_s("new_file", git_pathspec_match_list_entry(m, 2));
	cl_assert_equal_s("staged_changes_modified_file", git_pathspec_match_list_entry(m, 3));
	cl_assert_equal_s("staged_delete_modified_file", git_pathspec_match_list_entry(m, 4));
	cl_assert_equal_s("staged_new_file", git_pathspec_match_list_entry(m, 5));
	cl_assert_equal_s("staged_new_file_modified_file", git_pathspec_match_list_entry(m, 6));
	cl_assert_equal_s(NULL, git_pathspec_match_list_entry(m, 7));

	git_pathspec_match_list_free(m);

	git_pathspec_free(ps);
}


// Source: pathspec.c
// Lines 125-156
