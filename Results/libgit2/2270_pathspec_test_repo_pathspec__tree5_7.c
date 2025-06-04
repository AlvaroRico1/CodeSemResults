void test_repo_pathspec__tree5(void)
{
	git_object *tree;
	git_strarray s;
	git_pathspec *ps;
	git_pathspec_match_list *m;

	/* { "S*" } */
	s.strings = str5; s.count = ARRAY_SIZE(str5);
	cl_git_pass(git_pathspec_new(&ps, &s));

	cl_git_pass(git_revparse_single(&tree, g_repo, "HEAD~2^{tree}"));

	cl_git_pass(git_pathspec_match_tree(&m, (git_tree *)tree,
		GIT_PATHSPEC_USE_CASE | GIT_PATHSPEC_FIND_FAILURES, ps));
	cl_assert_equal_sz(0, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_sz(1, git_pathspec_match_list_failed_entrycount(m));
	git_pathspec_match_list_free(m);

	cl_git_pass(git_pathspec_match_tree(&m, (git_tree *)tree,
		GIT_PATHSPEC_IGNORE_CASE | GIT_PATHSPEC_FIND_FAILURES, ps));
	cl_assert_equal_sz(5, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_s("staged_changes", git_pathspec_match_list_entry(m, 0));
	cl_assert_equal_s("staged_delete_modified_file", git_pathspec_match_list_entry(m, 4));
	cl_assert_equal_sz(0, git_pathspec_match_list_failed_entrycount(m));
	git_pathspec_match_list_free(m);

	git_object_free(tree);

	cl_git_pass(git_revparse_single(&tree, g_repo, "HEAD^{tree}"));

	cl_git_pass(git_pathspec_match_tree(&m, (git_tree *)tree,
		GIT_PATHSPEC_IGNORE_CASE | GIT_PATHSPEC_FIND_FAILURES, ps));
	cl_assert_equal_sz(9, git_pathspec_match_list_entrycount(m));
	cl_assert_equal_s("staged_changes", git_pathspec_match_list_entry(m, 0));
	cl_assert_equal_s("subdir.txt", git_pathspec_match_list_entry(m, 5));
	cl_assert_equal_s("subdir/current_file", git_pathspec_match_list_entry(m, 6));
	cl_assert_equal_sz(0, git_pathspec_match_list_failed_entrycount(m));
	git_pathspec_match_list_free(m);

	git_object_free(tree);

	git_pathspec_free(ps);
}


// Source: pathspec.c
// Lines 317-360
