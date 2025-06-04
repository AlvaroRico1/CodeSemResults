void test_repo_open__format_version_1(void)
{
	git_repository *repo;
	git_config *config;

	repo = cl_git_sandbox_init("empty_bare.git");

	cl_git_pass(git_repository_open(&repo, "empty_bare.git"));
	cl_git_pass(git_repository_config(&config, repo));

	cl_git_pass(git_config_set_int32(config, "core.repositoryformatversion", 1));

	git_config_free(config);
	git_repository_free(repo);

	cl_git_pass(git_repository_open(&repo, "empty_bare.git"));
	cl_assert(git_repository_path(repo) != NULL);
	cl_assert(git__suffixcmp(git_repository_path(repo), "/") == 0);
	git_repository_free(repo);
}


// Source: open.c
// Lines 24-43
