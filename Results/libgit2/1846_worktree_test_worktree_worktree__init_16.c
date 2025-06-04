void test_worktree_worktree__init(void)
{
	git_worktree *wt;
	git_repository *repo;
	git_reference *branch;
	git_str path = GIT_STR_INIT;

	cl_git_pass(git_str_joinpath(&path, fixture.repo->workdir, "../worktree-new"));
	cl_git_pass(git_worktree_add(&wt, fixture.repo, "worktree-new", path.ptr, NULL));

	/* Open and verify created repo */
	cl_git_pass(git_repository_open(&repo, path.ptr));
	cl_assert(git__suffixcmp(git_repository_workdir(repo), "worktree-new/") == 0);
	cl_git_pass(git_branch_lookup(&branch, repo, "worktree-new", GIT_BRANCH_LOCAL));

	git_str_dispose(&path);
	git_worktree_free(wt);
	git_reference_free(branch);
	git_repository_free(repo);
}


// Source: worktree.c
// Lines 199-218
