void assert_config_entry_value(
	git_repository *repo,
	const char *name,
	const char *expected_value)
{
	git_config *config;
	git_buf buf = GIT_BUF_INIT;

	cl_git_pass(git_repository_config__weakptr(&config, repo));

	cl_git_pass(git_config_get_string_buf(&buf, config, name));

	cl_assert_equal_s(expected_value, buf.ptr);
	git_buf_dispose(&buf);
}


// Source: config_helpers.c
// Lines 25-39
