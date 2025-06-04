void test_config_write__to_file_with_only_comment(void)
{
	git_config *cfg;
	const char *filename = "config-file";
	git_str result = GIT_STR_INIT;

	cl_git_mkfile(filename, "\n\n");
	cl_git_pass(git_config_open_ondisk(&cfg, filename));
	cl_git_pass(git_config_set_string(cfg, "section.name", "value"));
	git_config_free(cfg);

	cl_git_pass(git_futils_readbuffer(&result, "config-file"));
	cl_assert_equal_s("\n\n[section]\n\tname = value\n", result.ptr);

	git_str_dispose(&result);
}


// Source: write.c
// Lines 624-639
