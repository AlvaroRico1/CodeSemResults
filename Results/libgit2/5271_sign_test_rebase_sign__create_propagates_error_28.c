void test_rebase_sign__create_propagates_error(void)
{
	git_rebase *rebase;
	git_reference *branch_ref, *upstream_ref;
	git_annotated_commit *branch_head, *upstream_head;
	git_oid commit_id;
	git_rebase_operation *rebase_operation;
	git_rebase_options rebase_opts = GIT_REBASE_OPTIONS_INIT;

	rebase_opts.commit_create_cb = create_cb_error;

	cl_git_pass(git_reference_lookup(&branch_ref, repo, "refs/heads/gravy"));
	cl_git_pass(git_reference_lookup(&upstream_ref, repo, "refs/heads/veal"));

	cl_git_pass(git_annotated_commit_from_ref(&branch_head, repo, branch_ref));
	cl_git_pass(git_annotated_commit_from_ref(&upstream_head, repo, upstream_ref));

	cl_git_pass(git_rebase_init(&rebase, repo, branch_head, upstream_head, NULL, &rebase_opts));

	cl_git_pass(git_rebase_next(&rebase_operation, rebase));
	cl_git_fail_with(GIT_EUSER, git_rebase_commit(&commit_id, rebase, NULL, signature, NULL, NULL));

	git_reference_free(branch_ref);
	git_reference_free(upstream_ref);
	git_annotated_commit_free(branch_head);
	git_annotated_commit_free(upstream_head);
	git_rebase_free(rebase);
}


// Source: sign.c
// Lines 222-249
