void cl_repo_set_string(git_repository *repo, const char *cfg, const char *value)
{
	git_config *config;
	cl_git_pass(git_repository_config(&config, repo));
	cl_git_pass(git_config_set_string(config, cfg, value));
	git_config_free(config);
}


// Source: clar_libgit2.c
// Lines 479-485
