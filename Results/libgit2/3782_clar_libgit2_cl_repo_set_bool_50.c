void cl_repo_set_bool(git_repository *repo, const char *cfg, int value)
{
	git_config *config;
	cl_git_pass(git_repository_config(&config, repo));
	cl_git_pass(git_config_set_bool(config, cfg, value != 0));
	git_config_free(config);
}


// Source: clar_libgit2.c
// Lines 460-466
