void test_config_read__foreach(void)
{
	git_config *cfg;
	int count, ret;

	cl_git_pass(git_config_new(&cfg));
	cl_git_pass(git_config_add_file_ondisk(cfg, cl_fixture("config/config9"),
		GIT_CONFIG_LEVEL_SYSTEM, NULL, 0));
	cl_git_pass(git_config_add_file_ondisk(cfg, cl_fixture("config/config15"),
		GIT_CONFIG_LEVEL_GLOBAL, NULL, 0));

	count = 0;
	cl_git_pass(git_config_foreach(cfg, count_cfg_entries_and_compare_levels, &count));
	cl_assert_equal_i(7, count);

	count = 3;
	cl_git_fail(ret = git_config_foreach(cfg, cfg_callback_countdown, &count));
	cl_assert_equal_i(-100, ret);

	git_config_free(cfg);
}


// Source: read.c
// Lines 319-339
