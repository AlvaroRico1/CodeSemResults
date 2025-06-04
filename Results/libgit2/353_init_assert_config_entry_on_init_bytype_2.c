static void assert_config_entry_on_init_bytype(
	const char *config_key, int expected_value, bool is_bare)
{
	git_config *config;
	int error, current_value;
	const char *repo_path = is_bare ?
		"config_entry/test.bare.git" : "config_entry/test.non.bare.git";

	cl_set_cleanup(&cleanup_repository, "config_entry");

	cl_git_pass(git_repository_init(&g_repo, repo_path, is_bare));

	cl_git_pass(git_repository_config(&config, g_repo));
	error = git_config_get_bool(&current_value, config, config_key);
	git_config_free(config);

	if (expected_value >= 0) {
		cl_assert_equal_i(0, error);
		cl_assert_equal_i(expected_value, current_value);
	} else {
		cl_assert_equal_i(expected_value, error);
	}
}


// Source: init.c
// Lines 193-215
