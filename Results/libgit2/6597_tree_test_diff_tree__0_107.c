void test_diff_tree__0(void)
{
	/* grabbed a couple of commit oids from the history of the attr repo */
	const char *a_commit = "605812a";
	const char *b_commit = "370fe9ec22";
	const char *c_commit = "f5b0af1fb4f5c";
	git_tree *c;

	g_repo = cl_git_sandbox_init("attr");

	cl_assert((a = resolve_commit_oid_to_tree(g_repo, a_commit)) != NULL);
	cl_assert((b = resolve_commit_oid_to_tree(g_repo, b_commit)) != NULL);
	cl_assert((c = resolve_commit_oid_to_tree(g_repo, c_commit)) != NULL);

	opts.context_lines = 1;
	opts.interhunk_lines = 1;


	cl_git_pass(git_diff_tree_to_tree(&diff, g_repo, a, b, &opts));

	cl_git_pass(git_diff_foreach(
		diff, diff_file_cb, diff_binary_cb, diff_hunk_cb, diff_line_cb, &expect));

	cl_assert_equal_i(5, expect.files);
	cl_assert_equal_i(2, expect.file_status[GIT_DELTA_ADDED]);
	cl_assert_equal_i(1, expect.file_status[GIT_DELTA_DELETED]);
	cl_assert_equal_i(2, expect.file_status[GIT_DELTA_MODIFIED]);

	cl_assert_equal_i(5, expect.hunks);

	cl_assert_equal_i(7 + 24 + 1 + 6 + 6, expect.lines);
	cl_assert_equal_i(1, expect.line_ctxt);
	cl_assert_equal_i(24 + 1 + 5 + 5, expect.line_adds);
	cl_assert_equal_i(7 + 1, expect.line_dels);

	git_diff_free(diff);
	diff = NULL;

	memset(&expect, 0, sizeof(expect));

	cl_git_pass(git_diff_tree_to_tree(&diff, g_repo, c, b, &opts));

	cl_git_pass(git_diff_foreach(
		diff, diff_file_cb, diff_binary_cb, diff_hunk_cb, diff_line_cb, &expect));

	cl_assert_equal_i(2, expect.files);
	cl_assert_equal_i(0, expect.file_status[GIT_DELTA_ADDED]);
	cl_assert_equal_i(0, expect.file_status[GIT_DELTA_DELETED]);
	cl_assert_equal_i(2, expect.file_status[GIT_DELTA_MODIFIED]);

	cl_assert_equal_i(2, expect.hunks);

	cl_assert_equal_i(8 + 15, expect.lines);
	cl_assert_equal_i(1, expect.line_ctxt);
	cl_assert_equal_i(1, expect.line_adds);
	cl_assert_equal_i(7 + 14, expect.line_dels);

	git_tree_free(c);
}


// Source: tree.c
// Lines 31-89
