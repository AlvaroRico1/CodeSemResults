void test_refs_revparse__issue_994(void)
{
	git_repository *repo;
	git_reference *head, *with_at;
	git_object *target;

	repo = cl_git_sandbox_init("testrepo.git");

	cl_assert_equal_i(GIT_ENOTFOUND,
		git_revparse_single(&target, repo, "origin/bim_with_3d@11296"));

	cl_assert_equal_i(GIT_ENOTFOUND,
		git_revparse_single(&target, repo, "refs/remotes/origin/bim_with_3d@11296"));


	cl_git_pass(git_repository_head(&head, repo));
	cl_git_pass(git_reference_create(
		&with_at,
		repo,
		"refs/remotes/origin/bim_with_3d@11296",
		git_reference_target(head),
		0,
		NULL));

	cl_git_pass(git_revparse_single(&target, repo, "origin/bim_with_3d@11296"));
	git_object_free(target);

	cl_git_pass(git_revparse_single(&target, repo, "refs/remotes/origin/bim_with_3d@11296"));
	git_object_free(target);

	git_reference_free(with_at);
	git_reference_free(head);
	cl_git_sandbox_cleanup();
}


// Source: revparse.c
// Lines 605-638
