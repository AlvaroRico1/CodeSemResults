void test_worktree_worktree__prune_locked(void)
{
	git_worktree_prune_options opts = GIT_WORKTREE_PRUNE_OPTIONS_INIT;
	git_worktree *wt;
	git_repository *repo;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_pass(git_worktree_lock(wt, NULL));

	opts.flags = GIT_WORKTREE_PRUNE_VALID;
	cl_git_fail(git_worktree_prune(wt, &opts));
	/* Assert the repository is still valid */
	cl_git_pass(git_repository_open_from_worktree(&repo, wt));

	opts.flags = GIT_WORKTREE_PRUNE_VALID|GIT_WORKTREE_PRUNE_LOCKED;
	cl_git_pass(git_worktree_prune(wt, &opts));

	git_worktree_free(wt);
	git_repository_free(repo);
}


// Source: worktree.c
// Lines 555-574
