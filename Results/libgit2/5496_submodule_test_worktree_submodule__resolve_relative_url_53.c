void test_worktree_submodule__resolve_relative_url(void)
{
	git_str wt_path = GIT_STR_INIT;
	git_buf sm_relative_path = GIT_BUF_INIT, wt_relative_path = GIT_BUF_INIT;
	git_repository *repo;
	git_worktree *wt;

	cl_git_pass(git_futils_mkdir("subdir", 0755, GIT_MKDIR_PATH));
	cl_git_pass(git_fs_path_prettify_dir(&wt_path, "subdir", NULL));
	cl_git_pass(git_str_joinpath(&wt_path, wt_path.ptr, "wt"));

	/* Open child repository, which is a submodule */
	cl_git_pass(git_repository_open(&child.repo, WORKTREE_CHILD));

	/* Create worktree of submodule repository */
	cl_git_pass(git_worktree_add(&wt, child.repo, "subdir", wt_path.ptr, NULL));
	cl_git_pass(git_repository_open_from_worktree(&repo, wt));

	cl_git_pass(git_submodule_resolve_url(&sm_relative_path, repo,
		    "../" WORKTREE_CHILD));
	cl_git_pass(git_submodule_resolve_url(&wt_relative_path, child.repo,
		    "../" WORKTREE_CHILD));

	cl_assert_equal_s(sm_relative_path.ptr, wt_relative_path.ptr);

	git_worktree_free(wt);
	git_repository_free(repo);
	git_str_dispose(&wt_path);
	git_buf_dispose(&sm_relative_path);
	git_buf_dispose(&wt_relative_path);
}


// Source: submodule.c
// Lines 62-92
