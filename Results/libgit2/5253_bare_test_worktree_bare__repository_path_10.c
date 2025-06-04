void test_worktree_bare__repository_path(void)
{
	git_worktree *wt;
	git_repository *wtrepo;

	cl_git_pass(git_worktree_add(&wt, g_repo, "name", WORKTREE_REPO, NULL));
	cl_assert_equal_s(git_worktree_path(wt), cl_git_sandbox_path(0, WORKTREE_REPO, NULL));

	cl_git_pass(git_repository_open(&wtrepo, WORKTREE_REPO));
	cl_assert_equal_s(git_repository_path(wtrepo), cl_git_sandbox_path(1, COMMON_REPO, "worktrees", "name", NULL));

	cl_assert_equal_s(git_repository_commondir(g_repo), git_repository_commondir(wtrepo));
	cl_assert_equal_s(git_repository_workdir(wtrepo), cl_git_sandbox_path(1, WORKTREE_REPO, NULL));

	git_repository_free(wtrepo);
	git_worktree_free(wt);
}


// Source: bare.c
// Lines 56-72
