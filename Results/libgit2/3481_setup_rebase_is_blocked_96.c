static int rebase_is_blocked(void)
{
	git_rebase *rebase = NULL;
	int error;

	git_reference *branch_ref, *upstream_ref;
	git_annotated_commit *branch_head, *upstream_head;

	cl_assert_equal_i(GIT_REPOSITORY_STATE_NONE, git_repository_state(repo));

	cl_git_pass(git_reference_lookup(&branch_ref, repo, "refs/heads/beef"));
	cl_git_pass(git_reference_lookup(&upstream_ref, repo, "refs/heads/master"));

	cl_git_pass(git_annotated_commit_from_ref(&branch_head, repo, branch_ref));
	cl_git_pass(git_annotated_commit_from_ref(&upstream_head, repo, upstream_ref));

	error = git_rebase_init(&rebase, repo, branch_head, upstream_head, NULL, NULL);

	git_annotated_commit_free(branch_head);
	git_annotated_commit_free(upstream_head);

	git_reference_free(branch_ref);
	git_reference_free(upstream_ref);
	git_rebase_free(rebase);

	return error;
}


// Source: setup.c
// Lines 550-576
