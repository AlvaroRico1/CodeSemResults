void test_worktree_refs__delete_succeeds_after_pruning_worktree(void)
{
	git_worktree_prune_options opts = GIT_WORKTREE_PRUNE_OPTIONS_INIT;
	git_reference *branch;
	git_worktree *worktree;

	opts.flags = GIT_WORKTREE_PRUNE_VALID;

	cl_git_pass(git_worktree_lookup(&worktree, fixture.repo, fixture.worktreename));
	cl_git_pass(git_worktree_prune(worktree, &opts));
	git_worktree_free(worktree);

	cl_git_pass(git_branch_lookup(&branch, fixture.repo,
		    "testrepo-worktree", GIT_BRANCH_LOCAL));
	cl_git_pass(git_branch_delete(branch));
	git_reference_free(branch);
}


// Source: refs.c
// Lines 119-135
