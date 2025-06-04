void test_refs_branches_upstream__retrieve_a_remote_tracking_reference_from_a_branch_with_no_remote_returns_GIT_ENOTFOUND(void)
{
	git_reference *head;
	git_repository *repository;
	git_commit *target;

	repository = cl_git_sandbox_init("testrepo.git");

	cl_git_pass(git_repository_head(&head, repository));
	cl_git_pass(git_reference_peel(((git_object **)&target), head, GIT_OBJECT_COMMIT));
	git_reference_free(head);

	assert_merge_and_or_remote_key_missing(repository, target, "remoteless");
	assert_merge_and_or_remote_key_missing(repository, target, "mergeless");
	assert_merge_and_or_remote_key_missing(repository, target, "mergeandremoteless");

	git_commit_free(target);

	cl_git_sandbox_cleanup();
}


// Source: upstream.c
// Lines 127-146
