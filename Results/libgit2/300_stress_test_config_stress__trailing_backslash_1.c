void test_config_stress__trailing_backslash(void)
{
	git_config *config;
	const char *path =  "C:\\iam\\some\\windows\\path\\";

	cl_assert(git_fs_path_exists("git-test-config"));
	cl_git_pass(git_config_open_ondisk(&config, TEST_CONFIG));
	cl_git_pass(git_config_set_string(config, "windows.path", path));
	git_config_free(config);

	cl_git_pass(git_config_open_ondisk(&config, TEST_CONFIG));
	assert_config_value(config, "windows.path", path);

	git_config_free(config);
}


// Source: stress.c
// Lines 88-102
