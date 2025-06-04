void test_config_write__can_set_an_empty_value(void)
{
	git_repository *repository;
	git_config *config;
	git_buf buf = {0};

	repository = cl_git_sandbox_init("testrepo.git");
	cl_git_pass(git_repository_config(&config, repository));

	cl_git_pass(git_config_set_string(config, "core.somevar", ""));
	cl_git_pass(git_config_get_string_buf(&buf, config, "core.somevar"));
	cl_assert_equal_s("", buf.ptr);

	git_buf_dispose(&buf);
	git_config_free(config);
	cl_git_sandbox_cleanup();
}


// Source: write.c
// Lines 479-495
