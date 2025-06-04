void test_worktree_worktree__init_submodule(void)
{
	git_repository *repo, *sm, *wt;
	git_worktree *worktree;
	git_str path = GIT_STR_INIT;

	cleanup_fixture_worktree(&fixture);
	repo = setup_fixture_submod2();

	cl_git_pass(git_str_joinpath(&path, repo->workdir, "sm_unchanged"));
	cl_git_pass(git_repository_open(&sm, path.ptr));
	cl_git_pass(git_str_joinpath(&path, repo->workdir, "../worktree/"));
	cl_git_pass(git_worktree_add(&worktree, sm, "repo-worktree", path.ptr, NULL));
	cl_git_pass(git_repository_open_from_worktree(&wt, worktree));

	cl_git_pass(git_fs_path_prettify_dir(&path, path.ptr, NULL));
	cl_assert_equal_s(path.ptr, wt->workdir);
	cl_git_pass(git_fs_path_prettify_dir(&path, sm->commondir, NULL));
	cl_assert_equal_s(sm->commondir, wt->commondir);

	cl_git_pass(git_str_joinpath(&path, sm->gitdir, "worktrees/repo-worktree/"));
	cl_assert_equal_s(path.ptr, wt->gitdir);

	git_str_dispose(&path);
	git_worktree_free(worktree);
	git_repository_free(sm);
	git_repository_free(wt);
}


// Source: worktree.c
// Lines 361-388
