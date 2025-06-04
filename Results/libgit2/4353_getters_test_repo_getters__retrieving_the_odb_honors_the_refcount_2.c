void test_repo_getters__retrieving_the_odb_honors_the_refcount(void)
{
	git_odb *odb;
	git_repository *repo;

	cl_git_pass(git_repository_open(&repo, cl_fixture("testrepo.git")));

	cl_git_pass(git_repository_odb(&odb, repo));
	cl_assert(((git_refcount *)odb)->refcount.val == 2);

	git_repository_free(repo);
	cl_assert(((git_refcount *)odb)->refcount.val == 1);

	git_odb_free(odb);
}


// Source: getters.c
// Lines 39-53
