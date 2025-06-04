void assert_on_branch(git_repository *repo, const char *branch)
{
	git_reference *head;
	git_str bname = GIT_STR_INIT;

	cl_git_pass(git_reference_lookup(&head, repo, GIT_HEAD_FILE));
	cl_assert_(git_reference_type(head) == GIT_REFERENCE_SYMBOLIC, branch);

	cl_git_pass(git_str_joinpath(&bname, "refs/heads", branch));
	cl_assert_equal_s(bname.ptr, git_reference_symbolic_target(head));

	git_reference_free(head);
	git_str_dispose(&bname);
}


// Source: checkout_helpers.c
// Lines 7-20
