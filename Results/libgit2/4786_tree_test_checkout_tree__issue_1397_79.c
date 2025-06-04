void test_checkout_tree__issue_1397(void)
{
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	const char *partial_oid = "8a7ef04";
	git_object *tree = NULL;

	test_checkout_tree__cleanup(); /* cleanup default checkout */

	g_repo = cl_git_sandbox_init("issue_1397");

	cl_repo_set_bool(g_repo, "core.autocrlf", true);

	cl_git_pass(git_revparse_single(&tree, g_repo, partial_oid));

	opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	cl_git_pass(git_checkout_tree(g_repo, tree, &opts));

	check_file_contents("./issue_1397/crlf_file.txt", "first line\r\nsecond line\r\nboth with crlf");

	git_object_free(tree);
}


// Source: tree.c
// Lines 759-780
