void test_worktree_worktree__add_no_checkout(void)
{
	git_worktree *wt;
	git_repository *wtrepo;
	git_index *index;
	git_str path = GIT_STR_INIT;
	git_worktree_add_options opts = GIT_WORKTREE_ADD_OPTIONS_INIT;

	opts.checkout_options.checkout_strategy = GIT_CHECKOUT_NONE;

	cl_git_pass(git_str_joinpath(&path, fixture.repo->workdir, "../worktree-no-checkout"));
	cl_git_pass(git_worktree_add(&wt, fixture.repo, "worktree-no-checkout", path.ptr, &opts));

	cl_git_pass(git_repository_open(&wtrepo, path.ptr));
	cl_git_pass(git_repository_index(&index, wtrepo));
	cl_assert_equal_i(git_index_entrycount(index), 0);

	git_str_dispose(&path);
	git_worktree_free(wt);
	git_index_free(index);
	git_repository_free(wtrepo);
}


// Source: worktree.c
// Lines 295-316
