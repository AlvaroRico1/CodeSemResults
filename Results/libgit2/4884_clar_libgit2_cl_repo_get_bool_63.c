int cl_repo_get_bool(git_repository *repo, const char *cfg)
{
	int val = 0;
	git_config *config;
	cl_git_pass(git_repository_config(&config, repo));
	if (git_config_get_bool(&val, config, cfg) < 0)
		git_error_clear();
	git_config_free(config);
	return val;
}


// Source: clar_libgit2.c
// Lines 468-477
