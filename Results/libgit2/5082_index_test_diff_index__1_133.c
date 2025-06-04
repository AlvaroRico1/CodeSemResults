void test_diff_index__1(void)
{
	/* grabbed a couple of commit oids from the history of the attr repo */
	const char *a_commit = "26a125ee1bf"; /* the current HEAD */
	const char *b_commit = "0017bd4ab1ec3"; /* the start */
	git_tree *a = resolve_commit_oid_to_tree(g_repo, a_commit);
	git_tree *b = resolve_commit_oid_to_tree(g_repo, b_commit);
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_diff *diff = NULL;
	diff_expects exp;

	cl_assert(a);
	cl_assert(b);

	opts.context_lines = 1;
	opts.interhunk_lines = 1;

	memset(&exp, 0, sizeof(exp));

	cl_git_pass(git_diff_tree_to_index(&diff, g_repo, a, NULL, &opts));

	cl_assert_equal_i(1, git_diff_foreach(
		diff, diff_stop_after_2_files, NULL, NULL, NULL, &exp) );

	cl_assert_equal_i(2, exp.files);

	git_diff_free(diff);
	diff = NULL;

	git_tree_free(a);
	git_tree_free(b);
}


// Source: index.c
// Lines 109-140
