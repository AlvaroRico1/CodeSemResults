void test_worktree_worktree__open(void)
{
	git_worktree *wt;
	git_repository *repo;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));

	cl_git_pass(git_repository_open_from_worktree(&repo, wt));
	cl_assert_equal_s(git_repository_workdir(repo),
		git_repository_workdir(fixture.worktree));

	git_repository_free(repo);
	git_worktree_free(wt);
}


// Source: worktree.c
// Lines 127-140
