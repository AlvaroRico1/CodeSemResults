int count_config_entries_match(git_repository *repo, const char *pattern)
{
	git_config *config;
	int how_many = 0;

	cl_git_pass(git_repository_config(&config, repo));

	cl_assert_equal_i(0, git_config_foreach_match(
		config,	pattern, count_config_entries_cb, &how_many));

	git_config_free(config);

	return how_many;
}


// Source: config_helpers.c
// Lines 54-67
