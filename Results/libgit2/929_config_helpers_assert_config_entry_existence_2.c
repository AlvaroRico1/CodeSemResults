void assert_config_entry_existence(
	git_repository *repo,
	const char *name,
	bool is_supposed_to_exist)
{
	git_config *config;
	git_config_entry *entry = NULL;
	int result;

	cl_git_pass(git_repository_config__weakptr(&config, repo));

	result = git_config_get_entry(&entry, config, name);
	git_config_entry_free(entry);

	if (is_supposed_to_exist)
		cl_git_pass(result);
	else
		cl_assert_equal_i(GIT_ENOTFOUND, result);
}


// Source: config_helpers.c
// Lines 5-23
