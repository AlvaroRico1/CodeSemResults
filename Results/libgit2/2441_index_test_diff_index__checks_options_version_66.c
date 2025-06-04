void test_diff_index__checks_options_version(void)
{
	const char *a_commit = "26a125ee1bf";
	git_tree *a = resolve_commit_oid_to_tree(g_repo, a_commit);
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_diff *diff = NULL;
	const git_error *err;

	opts.version = 0;
	cl_git_fail(git_diff_tree_to_index(&diff, g_repo, a, NULL, &opts));
	err = git_error_last();
	cl_assert_equal_i(GIT_ERROR_INVALID, err->klass);
	cl_assert_equal_p(diff, NULL);

	git_error_clear();
	opts.version = 1024;
	cl_git_fail(git_diff_tree_to_index(&diff, g_repo, a, NULL, &opts));
	err = git_error_last();
	cl_assert_equal_i(GIT_ERROR_INVALID, err->klass);
	cl_assert_equal_p(diff, NULL);

	git_tree_free(a);
}


// Source: index.c
// Lines 142-164
