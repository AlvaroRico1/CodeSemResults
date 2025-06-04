void test_config_write__preserve_case(void)
{
	const char *filename = "config-preserve-case";
	git_config *cfg;
	git_str result = GIT_STR_INIT;
	const char *expected = "[sOMe]\n" \
		"\tThInG = foo\n" \
		"\tOtheR = thing\n";

	cl_git_pass(git_config_open_ondisk(&cfg, filename));
	cl_git_pass(git_config_set_string(cfg, "sOMe.ThInG", "foo"));
	cl_git_pass(git_config_set_string(cfg, "SomE.OtheR", "thing"));
	git_config_free(cfg);

	cl_git_pass(git_config_open_ondisk(&cfg, filename));

	cl_git_pass(git_futils_readbuffer(&result, filename));
	cl_assert_equal_s(expected, result.ptr);
	git_str_dispose(&result);

	git_config_free(cfg);
}


// Source: write.c
// Lines 726-747
