void test_diff_tree__bare(void)
{
	const char *a_commit = "8496071c1b46c85";
	const char *b_commit = "be3563ae3f79";

	g_repo = cl_git_sandbox_init("testrepo.git");

	cl_assert((a = resolve_commit_oid_to_tree(g_repo, a_commit)) != NULL);
	cl_assert((b = resolve_commit_oid_to_tree(g_repo, b_commit)) != NULL);

	opts.context_lines = 1;
	opts.interhunk_lines = 1;

	cl_git_pass(git_diff_tree_to_tree(&diff, g_repo, a, b, &opts));

	cl_git_pass(git_diff_foreach(
		diff, diff_file_cb, diff_binary_cb, diff_hunk_cb, diff_line_cb, &expect));

	cl_assert_equal_i(3, expect.files);
	cl_assert_equal_i(2, expect.file_status[GIT_DELTA_ADDED]);
	cl_assert_equal_i(0, expect.file_status[GIT_DELTA_DELETED]);
	cl_assert_equal_i(1, expect.file_status[GIT_DELTA_MODIFIED]);

	cl_assert_equal_i(3, expect.hunks);

	cl_assert_equal_i(4, expect.lines);
	cl_assert_equal_i(0, expect.line_ctxt);
	cl_assert_equal_i(3, expect.line_adds);
	cl_assert_equal_i(1, expect.line_dels);
}


// Source: tree.c
// Lines 181-210
