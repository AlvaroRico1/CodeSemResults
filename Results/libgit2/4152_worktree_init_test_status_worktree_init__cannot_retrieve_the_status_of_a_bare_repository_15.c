void test_status_worktree_init__cannot_retrieve_the_status_of_a_bare_repository(void)
{
	git_repository *repo;
	unsigned int status = 0;

	cl_git_pass(git_repository_open(&repo, cl_fixture("testrepo.git")));
	cl_assert_equal_i(GIT_EBAREREPO, git_status_file(&status, repo, "dummy"));
	git_repository_free(repo);
}


// Source: worktree_init.c
// Lines 16-24
