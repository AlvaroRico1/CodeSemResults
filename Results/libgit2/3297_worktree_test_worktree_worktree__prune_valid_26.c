void test_worktree_worktree__prune_valid(void)
{
	git_worktree_prune_options opts = GIT_WORKTREE_PRUNE_OPTIONS_INIT;
	git_worktree *wt;
	git_repository *repo;

	opts.flags = GIT_WORKTREE_PRUNE_VALID;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_pass(git_worktree_prune(wt, &opts));

	/* Assert the repository is not valid anymore */
	cl_git_fail(git_repository_open_from_worktree(&repo, wt));

	git_worktree_free(wt);
	git_repository_free(repo);
}


// Source: worktree.c
// Lines 537-553
