void test_repo_open__with_symlinked_config(void)
{
#ifndef GIT_WIN32
	git_str path = GIT_STR_INIT;
	git_repository *repo;
	git_config *cfg;
	int32_t value;

	cl_git_sandbox_init("empty_standard_repo");

	/* Setup .gitconfig as symlink */
	cl_git_pass(git_futils_mkdir_r("home", 0777));
	cl_git_mkfile("home/.gitconfig.linked", "[global]\ntest = 4567\n");
	cl_must_pass(symlink(".gitconfig.linked", "home/.gitconfig"));
	cl_git_pass(git_fs_path_prettify(&path, "home", NULL));
	cl_git_pass(git_libgit2_opts(GIT_OPT_SET_SEARCH_PATH, GIT_CONFIG_LEVEL_GLOBAL, path.ptr));

	cl_git_pass(git_repository_open(&repo, "empty_standard_repo"));
	cl_git_pass(git_config_open_default(&cfg));
	cl_git_pass(git_config_get_int32(&value, cfg, "global.test"));
	cl_assert_equal_i(4567, value);

	git_config_free(cfg);
	git_repository_free(repo);
	cl_git_pass(git_futils_rmdir_r(git_str_cstr(&path), NULL, GIT_RMDIR_REMOVE_FILES));
	cl_sandbox_set_search_path_defaults();
	git_str_dispose(&path);
#endif
}


// Source: open.c
// Lines 136-164
