void test_refs_revparse__try_to_retrieve_sha_before_branch(void)
{
	git_repository *repo;
	git_reference *branch;
	git_object *target;
	char sha[GIT_OID_HEXSZ + 1];

	repo = cl_git_sandbox_init("testrepo.git");

	test_object_inrepo("a65fedf39aefe402d3bb6e24df4d4f5fe4547750", "a65fedf39aefe402d3bb6e24df4d4f5fe4547750", repo);

	cl_git_pass(git_revparse_single(&target, repo, "HEAD~3"));
	cl_git_pass(git_branch_create(&branch, repo, "a65fedf39aefe402d3bb6e24df4d4f5fe4547750", (git_commit *)target, 0));

	git_oid_tostr(sha, GIT_OID_HEXSZ + 1, git_object_id(target));

	test_object_inrepo("a65fedf39aefe402d3bb6e24df4d4f5fe4547750", "a65fedf39aefe402d3bb6e24df4d4f5fe4547750", repo);
	test_object_inrepo("heads/a65fedf39aefe402d3bb6e24df4d4f5fe4547750", sha, repo);

	git_reference_free(branch);
	git_object_free(target);
	cl_git_sandbox_cleanup();
}


// Source: revparse.c
// Lines 690-712
