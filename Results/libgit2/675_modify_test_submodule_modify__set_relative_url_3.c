void test_submodule_modify__set_relative_url(void)
{
	git_str path = GIT_STR_INIT;
	git_repository *repo;
	git_submodule *sm;

	cl_git_pass(git_submodule_set_url(g_repo, SM1, "../relative-url"));
	cl_git_pass(git_submodule_lookup(&sm, g_repo, SM1));
	cl_git_pass(git_submodule_sync(sm));
	cl_git_pass(git_submodule_open(&repo, sm));

	cl_git_pass(git_str_joinpath(&path, clar_sandbox_path(), "relative-url"));

	assert_config_entry_value(g_repo, "submodule."SM1".url", path.ptr);
	assert_config_entry_value(repo, "remote.origin.url", path.ptr);

	git_repository_free(repo);
	git_submodule_free(sm);
	git_str_dispose(&path);
}


// Source: modify.c
// Lines 214-233
