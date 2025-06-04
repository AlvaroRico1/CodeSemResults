void test_worktree_worktree__prune_without_opts_fails(void)
{
	git_worktree *wt;
	git_repository *repo;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_fail(git_worktree_prune(wt, NULL));

	/* Assert the repository is still valid */
	cl_git_pass(git_repository_open_from_worktree(&repo, wt));

	git_worktree_free(wt);
	git_repository_free(repo);
}


// Source: worktree.c
// Lines 522-535
