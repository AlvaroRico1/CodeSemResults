void test_config_configlevel__adding_the_same_level_twice_returns_EEXISTS(void)
{
	int error;
	git_config *cfg;

	cl_git_pass(git_config_new(&cfg));
	cl_git_pass(git_config_add_file_ondisk(cfg, cl_fixture("config/config9"),
		GIT_CONFIG_LEVEL_LOCAL, NULL, 0));
	cl_git_pass(git_config_add_file_ondisk(cfg, cl_fixture("config/config15"),
		GIT_CONFIG_LEVEL_GLOBAL, NULL, 0));
	error = git_config_add_file_ondisk(cfg, cl_fixture("config/config16"),
		GIT_CONFIG_LEVEL_GLOBAL, NULL, 0);

	cl_git_fail(error);
	cl_assert_equal_i(GIT_EEXISTS, error);

	git_config_free(cfg);
}


// Source: configlevel.c
// Lines 3-20
