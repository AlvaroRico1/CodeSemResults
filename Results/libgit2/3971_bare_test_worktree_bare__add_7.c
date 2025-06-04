void test_worktree_bare__add(void)
{
	git_worktree *wt;
	git_repository *wtrepo;
	git_strarray wts;

	cl_git_pass(git_worktree_add(&wt, g_repo, "name", WORKTREE_REPO, NULL));

	cl_git_pass(git_worktree_list(&wts, g_repo));
	cl_assert_equal_i(wts.count, 1);

	cl_git_pass(git_worktree_validate(wt));

	cl_git_pass(git_repository_open(&wtrepo, WORKTREE_REPO));
	cl_assert_equal_i(0, git_repository_is_bare(wtrepo));
	cl_assert_equal_i(1, git_repository_is_worktree(wtrepo));

	git_strarray_dispose(&wts);
	git_worktree_free(wt);
	git_repository_free(wtrepo);
}


// Source: bare.c
// Lines 34-54
